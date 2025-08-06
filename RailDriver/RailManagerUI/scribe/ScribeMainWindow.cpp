#include <ScribeMainWindow.h>
#include <utilityfunctions.h>
#include <ui_ScribeMainWindow.h>
#include <settings.h>                   // storing app state
#include <ParserLangHighlighter.h>
#include <QtDebug>
#include <QtPrintSupport/QPrinter>      // printing
#include <QtPrintSupport/QPrintDialog>  // printing
#include <QFileDialog>                  // file open/save dialogs
#include <QFile>                        // file descriptors, IO
#include <QTextStream>                  // file IO
#include <QStandardPaths>               // default open directory
#include <QDateTime>                    // current time
#include <QApplication>                 // quit
#include <QShortcut>
#include <QSerialPort>
#include <QThread>
#include <QSerialPortInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QShowEvent>
#include <parser.h>

/* Sets up the main application window and all of its children/widgets.
 */
ScribeMainWindow::ScribeMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::ScribeMainWindow)
{
    ui->setupUi(this);
    readSettings();

    // Used to ensure that only one language can ever be checked at a time
    languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);
    languageGroup->addAction(ui->actionC_Lang);
    languageGroup->addAction(ui->actionCPP_Lang);
    languageGroup->addAction(ui->actionJava_Lang);
    languageGroup->addAction(ui->actionPython_Lang);
    languageGroup->addAction(ui->actionParser_Lang);

    ui->actionParser_Lang->setChecked(true);

    connect(ui->actionParser_Lang, &QAction::triggered, this, &ScribeMainWindow::setParserLang);
    connect(languageGroup, SIGNAL(triggered(QAction*)), this, SLOT(on_languageSelected(QAction*)));

    // Language label frame
    setupLanguageOnStatusBar();

    // Set up the find dialog
    findDialog = new FindDialog();
    findDialog->setParent(this, Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);

    // Set up the goto dialog
    gotoDialog = new GotoDialog();
    gotoDialog->setParent(this, Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);

    // Set up the tabbed editor
    tabbedEditor  =ui->tabWidget;
    CommandResult =ui->CommandResult;
    CommandResult->clear();
    simpleCommand =ui->simpleCommand;
    ui->sendProgressBar->setVisible(false);
    tabbedEditor->setTabsClosable(true);

    // Add metric reporter and simulate a tab switch
    metricReporter = new MetricReporter();
    ui->statusBar->addPermanentWidget(metricReporter);
    on_currentTabChanged(0);
    setParserLang();


    // Connect tabbedEditor's signals to their handlers
    connect(tabbedEditor, SIGNAL(currentChanged(int)), this, SLOT(on_currentTabChanged(int)));
    connect(tabbedEditor, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    // Connect action signals to their handlers
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(on_actionSaveTriggered()));
    connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(on_actionSaveTriggered()));
    connect(ui->actionReplace, SIGNAL(triggered()), this, SLOT(on_actionFind_triggered()));
    connect(ui->actionSelect_Port_Com, &QAction::triggered, this, &ScribeMainWindow::actionSelect_Port_Com);
    connect(ui->stopTransmissionButton, &QPushButton::clicked,this, &ScribeMainWindow::on_stopTransmissionButton_clicked);
    connect(ui->clearCommandButton, &QPushButton::clicked,this, &ScribeMainWindow::on_clearCommandButton_clicked);
    connect(ui->simpleCommand,SIGNAL(returnPressed()),this,SLOT(on_simpleCommand()));

    // Have to add this shortcut manually because we can't define it via the GUI editor
    QShortcut *tabCloseShortcut = new QShortcut(QKeySequence("Ctrl+W"), this);
    QObject::connect(tabCloseShortcut, SIGNAL(activated()), this, SLOT(closeTabShortcut()));

    // For word wrap and auto indent
    matchFormatOptionsToEditorDefaults();

    mapMenuLanguageOptionToLanguageType();
    mapFileExtensionsToLanguages();
    appendShortcutsToToolbarTooltips();

    // Command line

    tabbedEditor->removeTab(0);

    // If we closed the last tab, make a new one
    if (tabbedEditor->count() == 0)
    {
        on_actionNew_triggered();
    }
}

void ScribeMainWindow::setParserLang() {
    if (!editor) return;
    editor->setProgrammingLanguage(ProgrammingLanguage::Language::Parser);
    editor->document()->markContentsDirty(0, editor->toPlainText().length());
}

void ScribeMainWindow::on_sendAgainButton_clicked() {
    on_simpleCommand();
}

void ScribeMainWindow::on_clearCommandButton_clicked() {
    ui->simpleCommand->clear();
    ui->simpleCommand->setFocus();
}
QString ScribeMainWindow::getCurrentDocument() const {
    return editor->getContent();
}
QString ScribeMainWindow::getCurrentLanguage() const {
    return toString(editor->getProgrammingLanguage());
}

/* Ensures that the checkable formatting menu options, like auto indent
 * and word wrap, match the previously saved defaults for the Editor class.
 * See constructor for usage.
 */
void ScribeMainWindow::matchFormatOptionsToEditorDefaults()
{
    QAction *autoIndent = ui->actionAuto_Indent;
    editor->autoIndentEnabled ? autoIndent->setChecked(true) : autoIndent->setChecked(false);

    QAction *wordWrap = ui->actionWord_Wrap;
    editor->lineWrapMode ? wordWrap->setChecked(true) : wordWrap->setChecked(false);
}

/* Updates the Format menu options (e.g., Word wrap, Auto indent) to match
 * the settings of the currently selected editor. See onCurrentTabChanged for usage.
 */
void ScribeMainWindow::updateFormatMenuOptions()
{
    ui->actionWord_Wrap->setChecked(editor->textIsWrapped());
    ui->actionAuto_Indent->setChecked(editor->textIsAutoIndented());
}

/* Initializes the language label and adds it to a frame
 * that gets set as a widget on the far left of the status bar.
 */
void ScribeMainWindow::setupLanguageOnStatusBar()
{
    languageLabel = new QLabel("Language: not selected");
    QFrame *langFrame = new QFrame();
    QHBoxLayout *langLayout = new QHBoxLayout();
    langLayout->addWidget(languageLabel);
    langFrame->setLayout(langLayout);
    ui->statusBar->addWidget(langFrame);
}

/* Maps each menu language option (from the Format dropdown) to its corresponding
 * Language type, for convenience.
 */
void ScribeMainWindow::mapMenuLanguageOptionToLanguageType()
{
    menuActionToLanguageMap[ui->actionC_Lang] = Language::C;
    menuActionToLanguageMap[ui->actionCPP_Lang] = Language::CPP;
    menuActionToLanguageMap[ui->actionJava_Lang] = Language::Java;
    menuActionToLanguageMap[ui->actionPython_Lang] = Language::Python;
    menuActionToLanguageMap[ui->actionParser_Lang] = Language::Parser;

}

/* Maps known file extensions to the languages the editor supports.
 */
void ScribeMainWindow::mapFileExtensionsToLanguages()
{
    extensionToLanguageMap.insert("cpp", Language::CPP);
    extensionToLanguageMap.insert("h", Language::CPP);
    extensionToLanguageMap.insert("c", Language::C);
    extensionToLanguageMap.insert("java", Language::Java);
    extensionToLanguageMap.insert("py", Language::Python);
    extensionToLanguageMap.insert("txt", Language::Parser);

}

void ScribeMainWindow::appendShortcutsToToolbarTooltips()
{
    for (QAction* action : ui->mainToolBar->actions())
    {
        QString tooltip = action->toolTip() + " (" + action->shortcut().toString() + ")";
        action->setToolTip(tooltip);
    }
}

/* Performs all necessary memory cleanup operations on dynamically allocated objects.
 */
ScribeMainWindow::~ScribeMainWindow()
{
    delete languageLabel;
    delete languageGroup;
    delete ui;
}


/* Called when the user selects a language from the main menu. Sets the current language to
 * that language internally for the currently tabbed Editor.
 */
void ScribeMainWindow::on_languageSelected(QAction* languageAction)
{
    Language language = menuActionToLanguageMap[languageAction];
    selectProgrammingLanguage(language);
}


/* Given a Language enum, this function checks the corresponding radio option from the Format > Language
 * menu. Used by on_currentTabChanged to reflect the current tab's selected language.
 */
void ScribeMainWindow::triggerCorrespondingMenuLanguageOption(Language lang)
{
    switch (lang)
    {
        case (Language::C):
            if (!ui->actionC_Lang->isChecked())
            {
                ui->actionC_Lang->trigger();
            }
            break;

        case (Language::CPP):
            if (!ui->actionCPP_Lang->isChecked())
            {
                ui->actionCPP_Lang->trigger();
            }
            break;

        case (Language::Java):
            if (!ui->actionJava_Lang->isChecked())
            {
                ui->actionJava_Lang->trigger();
            }
            break;

        case (Language::Python):
            if (!ui->actionPython_Lang->isChecked())
            {
                ui->actionPython_Lang->trigger();
            }
            break;

        case (Language::Parser):
            if (!ui->actionParser_Lang->isChecked())
            {
                ui->actionParser_Lang->trigger();
            }
            break;


        default: return;
    }
}


/* Uses the extension of a file to determine what language, if any, it should be
 * mapped to. If the extension does not match one of the supported languages, or if
 * the file does not have an extension, then the language is set to Language::None.
 */
void ScribeMainWindow::setLanguageFromExtension()
{
    QString fileName = editor->getFileName();
    int indexOfDot = fileName.indexOf('.');

    if (indexOfDot == -1)
    {
        selectProgrammingLanguage(Language::None);
        return;
    }

    QString fileExtension = fileName.mid(indexOfDot + 1);

    bool extensionSupported = extensionToLanguageMap.find(fileExtension) != extensionToLanguageMap.end();

    if (!extensionSupported)
    {
        selectProgrammingLanguage(Language::None);
        return;
    }

    selectProgrammingLanguage(extensionToLanguageMap[fileExtension]);
}


/* Wrapper for all common logic that needs to run whenever a given language
 * is selected for use on a particular tab. Triggers the corresponding menu option.
 */
void ScribeMainWindow::selectProgrammingLanguage(Language language)
{
    if (language == editor->getProgrammingLanguage())
    {
        return;
    }

    editor->setProgrammingLanguage(language);
    languageLabel->setText(toString(language));
    triggerCorrespondingMenuLanguageOption(language);
}


/* Disconnects all signals that depend on the cached editor/tab. Used mainly
 * when the current editor is changed (when a new tab is opened, for example).
 */
void ScribeMainWindow::disconnectEditorDependentSignals()
{
    disconnect(findDialog, SIGNAL(startFinding(QString, bool, bool)), editor, SLOT(find(QString, bool, bool)));
    disconnect(findDialog, SIGNAL(startReplacing(QString, QString, bool, bool)), editor, SLOT(replace(QString, QString, bool, bool)));
    disconnect(findDialog, SIGNAL(startReplacingAll(QString, QString, bool, bool)), editor, SLOT(replaceAll(QString, QString, bool, bool)));
    disconnect(gotoDialog, SIGNAL(gotoLine(int)), editor, SLOT(goTo(int)));
    disconnect(editor, SIGNAL(findResultReady(QString)), findDialog, SLOT(onFindResultReady(QString)));
    disconnect(editor, SIGNAL(gotoResultReady(QString)), gotoDialog, SLOT(onGotoResultReady(QString)));

    disconnect(editor, SIGNAL(wordCountChanged(int)), metricReporter, SLOT(updateWordCount(int)));
    disconnect(editor, SIGNAL(charCountChanged(int)), metricReporter, SLOT(updateCharCount(int)));
    disconnect(editor, SIGNAL(lineCountChanged(int, int)), metricReporter, SLOT(updateLineCount(int, int)));
    disconnect(editor, SIGNAL(columnCountChanged(int)), metricReporter, SLOT(updateColumnCount(int)));
    disconnect(editor, SIGNAL(fileContentsChanged()), this, SLOT(updateTabAndWindowTitle()));

    disconnect(editor, SIGNAL(undoAvailable(bool)), this, SLOT(toggleUndo(bool)));
    disconnect(editor, SIGNAL(redoAvailable(bool)), this, SLOT(toggleRedo(bool)));
    disconnect(editor, SIGNAL(copyAvailable(bool)), this, SLOT(toggleCopyAndCut(bool)));
}


/* Connects all signals and slots that depend on the cached editor/tab. Used mainly
 * when the current editor is changed (when a new tab is opened, for example).
 */
void ScribeMainWindow::reconnectEditorDependentSignals()
{
    connect(findDialog, SIGNAL(startFinding(QString, bool, bool)), editor, SLOT(find(QString, bool, bool)));
    connect(findDialog, SIGNAL(startReplacing(QString, QString, bool, bool)), editor, SLOT(replace(QString, QString, bool, bool)));
    connect(findDialog, SIGNAL(startReplacingAll(QString, QString, bool, bool)), editor, SLOT(replaceAll(QString, QString, bool, bool)));
    connect(gotoDialog, SIGNAL(gotoLine(int)), editor, SLOT(goTo(int)));
    connect(editor, SIGNAL(findResultReady(QString)), findDialog, SLOT(onFindResultReady(QString)));
    connect(editor, SIGNAL(gotoResultReady(QString)), gotoDialog, SLOT(onGotoResultReady(QString)));

    connect(editor, SIGNAL(wordCountChanged(int)), metricReporter, SLOT(updateWordCount(int)));
    connect(editor, SIGNAL(charCountChanged(int)), metricReporter, SLOT(updateCharCount(int)));
    connect(editor, SIGNAL(lineCountChanged(int, int)), metricReporter, SLOT(updateLineCount(int, int)));
    connect(editor, SIGNAL(columnCountChanged(int)), metricReporter, SLOT(updateColumnCount(int)));
    connect(editor, SIGNAL(fileContentsChanged()), this, SLOT(updateTabAndWindowTitle()));

    connect(editor, SIGNAL(undoAvailable(bool)), this, SLOT(toggleUndo(bool)));
    connect(editor, SIGNAL(redoAvailable(bool)), this, SLOT(toggleRedo(bool)));
    connect(editor, SIGNAL(copyAvailable(bool)), this, SLOT(toggleCopyAndCut(bool)));
}

void ScribeMainWindow::onTabRegainedFocus()
{
    // Exemple : rafraîchir l’interface, relancer un processus, etc.
    ui->statusBar->showMessage("Éditeur actif", 2000);
}

void ScribeMainWindow::onTabLostFocus()
{
    editor = tabbedEditor->currentTab();
    emit checkTextSignal();

    // Exemple : mettre en pause des opérations, cacher des vues, etc.
}


/* Called each time the current tab changes in the tabbed editor. Sets the main window's current editor,
 * reconnects any relevant signals, and updates the window.
 */
void ScribeMainWindow::on_currentTabChanged(int index)
{
    // Happens when the tabbed editor's last tab is closed

    if (index == -1)
    {
        return;
    }

    // Note: editor will only be nullptr on the first launch, so this will get skipped in that edge case
    if (editor != nullptr)
    {
        disconnectEditorDependentSignals();
    }

    editor = tabbedEditor->currentTab();
    emit checkTextSignal();

    reconnectEditorDependentSignals();
    editor->setFocus(Qt::FocusReason::TabFocusReason);

    Language tabLanguage = editor->getProgrammingLanguage();

    // If this tab had a programming language set, trigger the corresponding option
    if (tabLanguage != Language::None)
    {
        triggerCorrespondingMenuLanguageOption(tabLanguage);
    }
    else
    {        
        // If a menu language is checked but the current tab has no language set, uncheck the menu option
        if (languageGroup->checkedAction()){
            languageGroup->checkedAction()->setChecked(false);
        }
    }

    // Update language reflected on status bar
    languageLabel->setText(toString(tabLanguage));

    // Update main window actions to reflect the current tab's available actions
    toggleRedo(editor->redoAvailable());
    toggleUndo(editor->undoAvailable());
    toggleCopyAndCut(editor->textCursor().hasSelection());

    updateFormatMenuOptions();

    // We need to update this information manually for tab changes
    DocumentMetrics metrics = editor->getDocumentMetrics();
    updateTabAndWindowTitle();
    metricReporter->updateWordCount(metrics.wordCount);
    metricReporter->updateCharCount(metrics.charCount);
    metricReporter->updateLineCount(metrics.currentLine, metrics.totalLines);
    metricReporter->updateColumnCount(metrics.currentColumn);
}


/* Launches the Find dialog box if it isn't already visible and sets its focus.
 */
void ScribeMainWindow::launchFindDialog()
{
    if (findDialog->isHidden())
    {
        findDialog->show();
        findDialog->activateWindow();
        findDialog->raise();
        findDialog->setFocus();
    }
}


/* Launches the Go To dialog box if it isn't already visible and sets its focus.
 */
void ScribeMainWindow::launchGotoDialog()
{
    if (gotoDialog->isHidden())
    {
        gotoDialog->show();
        gotoDialog->activateWindow();
        gotoDialog->raise();
        gotoDialog->setFocus();
    }
}


/* Updates the tab name and the main application window title to reflect the
 * currently open document.
 */
void ScribeMainWindow::updateTabAndWindowTitle()
{
    QString tabTitle = editor->getFileName();
    QString windowTitle = tabTitle;

    if (editor->isUnsaved())
    {
        tabTitle += " *";
        windowTitle += " [Unsaved]";
    }

    tabbedEditor->setTabText(tabbedEditor->currentIndex(), tabTitle);
    setWindowTitle(windowTitle + " - Scribe");
}


/* Launches a dialog box asking the user if they would like to save the current file.
 * If the user selects "No" or closes the dialog window, the file will not be saved.
 * Otherwise, if they select "Yes," the file will be saved.
 */
QMessageBox::StandardButton ScribeMainWindow::askUserToSave()
{
    QString fileName = editor->getFileName();

    return Utility::promptYesOrNo(this, "Unsaved changes", tr("Do you want to save the changes to ") + fileName + tr("?"));
}


/* Called when the user selects the New option from the menu or toolbar (or uses Ctrl+N).
 * Adds a new tab to the editor.
 */
void ScribeMainWindow::on_actionNew_triggered()
{
    tabbedEditor->add(new Editor());
}


/* Called when the user selects the Save or Save As option from the menu or toolbar
 * (or uses Ctrl+S). On success, saves the contents of the text editor to the disk using
 * the file name provided by the user. If the current document was never saved, or if the
 * user chose Save As, the program prompts the user to specify a name and directory for the file.
 * Returns true if the file was saved and false otherwise.
 */
bool ScribeMainWindow::on_actionSaveTriggered()
{
    bool saveAs = sender() == ui->actionSave_As;
    QString currentFilePath = editor->getCurrentFilePath();

    // If user hit Save As or user hit Save but current document was never saved to disk
    if (saveAs || currentFilePath.isEmpty())
    {
        // Title to be used for saving dialog
        QString saveDialogWindowTitle = saveAs ? tr("Save As") : tr("Save");

        // Try to get a valid file path
        QString filePath = QFileDialog::getSaveFileName(this, saveDialogWindowTitle);

        // Don't do anything if the user changes their mind and hits Cancel
        if (filePath.isNull())
        {
            return false;
        }
        editor->setCurrentFilePath(filePath);
    }

    // Attempt to create a file descriptor with the given path
    QFile file(editor->getCurrentFilePath());
    if (!file.open(QIODevice::WriteOnly | QFile::Text))
    {
        QMessageBox::critical(this, "Warning", "Cannot save file: " + file.errorString());
        return false;
    }

    ui->statusBar->showMessage("Document saved", 2000);

    // Save the contents of the editor to the disk and close the file descriptor
    QTextStream out(&file);
    QString editorContents = editor->toPlainText();
    out << editorContents;
    file.close();

    editor->setModifiedState(false);
    updateTabAndWindowTitle();
    setLanguageFromExtension();

    return true;
}


/* Called when the user selects the Open option from the menu or toolbar
 * (or uses Ctrl+O). If the current document has unsaved changes, it first
 * asks the user if they want to save. In any case, it launches a dialog box
 * that allows the user to select the file they want to open. Sets the editor's
 * current file path to that of the opened file on success and updates the app state.
 */
void ScribeMainWindow::on_actionOpen_triggered()
{
    // Used to switch to a new tab if there's already an open doc
    bool openInCurrentTab = editor->isUntitled() && !editor->isUnsaved();

    QString openedFilePath;
    QString lastUsedDirectory = settings->value(DEFAULT_DIRECTORY_KEY).toString();

    if (lastUsedDirectory.isEmpty())
    {
        openedFilePath = QFileDialog::getOpenFileName(this, tr("Open"), DEFAULT_DIRECTORY);
    }
    else
    {
        openedFilePath = QFileDialog::getOpenFileName(this, tr("Open"), lastUsedDirectory);
    }

    // Don't do anything if the user hit Cancel
    if (openedFilePath.isNull())
    {
        return;
    }

    // Update the recently used directory
    QDir currentDirectory;
    settings->setValue(DEFAULT_DIRECTORY_KEY, currentDirectory.absoluteFilePath(openedFilePath));

    // Attempt to create a file descriptor for the file at the given path
    QFile file(openedFilePath);
    if (!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(this, "Warning", "Cannot save file: " + file.errorString());
        return;
    }

    // Read the file contents into the editor and close the file descriptor
    QTextStream in(&file);
    QString documentContents = in.readAll();

    if (!openInCurrentTab)
    {
        tabbedEditor->add(new Editor());
    }
    editor->setCurrentFilePath(openedFilePath);
    editor->setPlainText(documentContents);
    file.close();

    editor->setModifiedState(false);
    updateTabAndWindowTitle();
    setLanguageFromExtension();
}

void ScribeMainWindow::processError(const QString &s)
{
    informUser(QString("Error"), s);
}

void ScribeMainWindow::processTimeout(const QString &s)
{
    informUser(QString("Error"), s);
}

void ScribeMainWindow::actionSelect_Port_Com() {
    // Récupère la liste des ports disponibles
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    if (ports.isEmpty()) {
        QMessageBox::critical(this, "Aucun port COM", "Aucun port COM disponible n’a été détecté.");
        return;
    }

    if (serial){
        serial->close();
        disconnect(serial, &QSerialPort::readyRead, this, &ScribeMainWindow::onDataReceived);
        delete serial;
        serial = Q_NULLPTR;
    }

    // Crée une liste de noms pour l'affichage
    QStringList portNames;
    for (const QSerialPortInfo &port : ports) {
        portNames << QString("%1 (%2)").arg(port.portName(), port.description());
    }

    // Affiche une boîte de dialogue pour choisir un port
    bool ok;
    QString selectedPort = QInputDialog::getItem(this,
                                                 "Sélection du port COM",
                                                 "Choisissez un port COM disponible :",
                                                 portNames,
                                                 0, // index sélectionné par défaut
                                                 false, // non modifiable
                                                 &ok);
    if (ok && !selectedPort.isEmpty()) {
        // Extrait uniquement le nom du port (avant le 1er espace ou parenthèse)
        gl_currentComPort = selectedPort.section(' ', 0, 0);
        openSerial();
    }
}

QString ScribeMainWindow::analyseCANinput(unsigned char* request){

    QString trameAnalysis;

    // CHECK EVENT FIRST
    if (request[REQ_EVENT_REQUEST_TRACK_EVENT] == TRUE) {
        trameAnalysis = QString("TRACK EVENT BOARD ");
        trameAnalysis.append(QString::number(request[REQ_EVENT_REQUEST_EVENT_BOARD_TRACK_NUMBER]));
        trameAnalysis.append(" TRACK ");
        trameAnalysis.append(QString::number(request[REQ_EVENT_REQUEST_EVENT_TRACK_NUMBER]));
        trameAnalysis.append(" ");
        trameAnalysis.append(request[REQ_EVENT_REQUEST_EVENT_VEHICLE_STATUS]==ONTRACKValue ? QString("ONTRACK"):QString("OFFTRACK"));
        return(trameAnalysis);
    }
    else if (request[REQ_EVENT_REQUEST_GPIO_EVENT] == TRUE) {
        trameAnalysis = QString("GPIO EVENT BOARD ");
        trameAnalysis.append(QString::number(request[REQ_EVENT_REQUEST_EVENT_BOARD_GPIO_NUMBER]));
        trameAnalysis.append(" GPIO ");
        trameAnalysis.append(QString::number(request[REQ_EVENT_REQUEST_EVENT_GPIO_NUMBER]));
        trameAnalysis.append(" LEVEL ");
        trameAnalysis.append(QString::number(request[REQ_EVENT_REQUEST_EVENT_GPIO_LEVEL]));
        return(trameAnalysis);
    }

    else if (request[REQ_EVENT_REQUEST_TIMER_EVENT] == TRUE) {
        trameAnalysis = QString("TIMER EVENT BOARD ");
        trameAnalysis.append(QString::number(request[REQ_EVENT_REQUEST_EVENT_BOARD_TIMER_NUMBER]));
        trameAnalysis.append(" TIMER ");
        trameAnalysis.append(QString::number(request[REQ_EVENT_REQUEST_EVENT_TIMER_NUMBER]));
        trameAnalysis.append(" TRIGGERED");
        return(trameAnalysis);
    }


    //Global command
    switch (request[REQ_GLOBAL_COMMAND]) {
    case STOPValue:
        trameAnalysis = QString("STOP ALL");
        return(trameAnalysis);
    case RUNALLValue :
        trameAnalysis = QString("RUN ALL");
        return(trameAnalysis);
    case RUNValue :
        trameAnalysis = QString("RUN ");
        trameAnalysis.append(QString::number(request[REQ_BOARD_NUMBER]));
        return(trameAnalysis);
    case CALIBValue :
        trameAnalysis = QString("CALIB ");
        trameAnalysis.append(QString::number(request[REQ_BOARD_NUMBER]));
        return(trameAnalysis);
    case RESETValue :
        trameAnalysis = QString("RESET ");
        trameAnalysis.append(QString::number(request[REQ_BOARD_NUMBER]));
        return(trameAnalysis);
    default :
        switch(request[REQ_TYPE_ENTRY]) {
        case PROGValue:
            trameAnalysis = QString("PROG ");
            trameAnalysis.append(QString::number(request[REQ_BOARD_NUMBER]));

            if (request[REQ_PROGRAM_REQUEST_SET_BOARD_MODE]==TRUE) {
                if(request[REQ_PROGRAM_REQUEST_BOARD_MODE]==DCCValue) {
                    trameAnalysis.append(" DCC");
                    return(trameAnalysis);
                }
                else if(request[REQ_PROGRAM_REQUEST_BOARD_MODE]==ANAValue) {
                    trameAnalysis.append(" ANA");
                    return(trameAnalysis);
                }
                else {
                    trameAnalysis = QString("PROG ");
                    trameAnalysis.append(QString::number(request[REQ_BOARD_NUMBER]));
                    trameAnalysis.append( " MODE_MISSING");
                    return(trameAnalysis);
                }
            }
            if (request[REQ_PROGRAM_REQUEST_SET_GPIO] == TRUE) {
                if (request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR]==INValue || request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR]==OUTValue) {
                    if (request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER]>=0 && request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER]<=3) {
                        trameAnalysis.append(QString(" GPIO "));
                        trameAnalysis.append(QString::number(request[REQ_PROGRAM_REQUEST_SET_GPIO_NUMBER]));
                        trameAnalysis.append(QString(" "));
                        trameAnalysis.append(request[REQ_PROGRAM_REQUEST_SET_GPIO_DIR] == INValue ? QString("IN"):QString("OUT"));
                        return(trameAnalysis);
                    }
                    else {
                        trameAnalysis.append(" GPIO BAD_GPIO_NUMBER");
                        return(trameAnalysis);
                    }
                }
                else {
                    trameAnalysis.append(" GPIO BAD_GPIO_DIR");
                    return(trameAnalysis);
                }
            }
            if (request[REQ_PROGRAM_REQUEST_SET_AUTOMATION] == TRUE) {
                trameAnalysis.append(QString(" AUT "));
                trameAnalysis.append(QString::fromUtf8(reinterpret_cast<const char*>(&request[REQ_PROGRAM_REQUEST_IDENT])));
            }
            if (request[REQ_PROGRAM_REQUEST_DEL_AUTOMATION] == TRUE) {
                trameAnalysis.append(" DEL ");
                trameAnalysis.append(QString::number(request[REQ_PROGRAM_REQUEST_AUTOMATION_NUMBER]));
                return(trameAnalysis);
            }

            break;

        case COMValue :
            trameAnalysis = QString("COM ").append(QString::number(request[REQ_BOARD_NUMBER]));

            if (request[REQ_COMMAND_REQUEST_SET_GPIO]==TRUE) {
                trameAnalysis.append(QString(" GPIO "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_GPIO_NUMBER]));
                trameAnalysis.append(QString(" "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_GPIO_LEVEL]));
                return(trameAnalysis);

            }
            else if (request[REQ_COMMAND_REQUEST_SET_TIMER]==TRUE) {
                trameAnalysis.append(QString(" TIMER "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_TIMER_NUMBER]));
                trameAnalysis.append(QString(" "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_TIMER_DELAY]));
                return(trameAnalysis);

            }
            else if (request[REQ_COMMAND_REQUEST_SET_LPO] == TRUE) {
                trameAnalysis.append(QString(" LPO "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_LPO_NUMBER]));
                trameAnalysis.append(QString(" "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_LPO_LEVEL]));
                return(trameAnalysis);

            }
            else if (request[REQ_COMMAND_REQUEST_SET_AUT] == TRUE) {
                trameAnalysis.append(request[REQ_COMMAND_REQUEST_AUT_STATUS]==AUTONValue ? QString("AUTON ") : QString("AUTOFF "));
                trameAnalysis.append(QString::fromUtf8(reinterpret_cast<const char*>(&request[REQ_COMMAND_REQUEST_AUT_IDENT])));

                return(trameAnalysis);
            }
            else if (request[REQ_COMMAND_REQUEST_SET_TRACK] == TRUE) {
                trameAnalysis.append(QString(" TRACK "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_TRACK_NUMBER]));
                trameAnalysis.append(QString(" SPEED "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_TRACK_SPEED]));
                trameAnalysis.append(request[REQ_COMMAND_REQUEST_TRACK_DIR]==FORWValue ? QString( "FORW "):QString(" BACK "));
                trameAnalysis.append(QString(" INERTIA "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_TRACK_INERTIA]));
                return(trameAnalysis);

            }
            else if (request[REQ_COMMAND_REQUEST_SET_USER_MODE]== TRUE) {
                if (request[REQ_COMMAND_REQUEST_USER_MODE]==MANUALValue){
                    trameAnalysis.append(QString(" MANUAL "));
                    return(trameAnalysis);
                }
                else if (request[REQ_COMMAND_REQUEST_USER_MODE]==MANUAL0Value){
                    trameAnalysis.append(QString(" MANUAL0 "));
                    return(trameAnalysis);
                }
                else if (request[REQ_COMMAND_REQUEST_USER_MODE]==AUTOMATICValue) {
                     trameAnalysis.append(QString(" AUTOMATIC"));
                    return(trameAnalysis);
                }
                else {
                    trameAnalysis.append(QString(" BAD_AUT_STATUS"));
                    return(trameAnalysis);
                }
            }
            else if (request[REQ_COMMAND_REQUEST_SET_DCC]== TRUE) {
                trameAnalysis.append(QString(" DCC "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_DCC_ADDRESS]));
                trameAnalysis.append(QString(" SPEED "));
                trameAnalysis.append(QString::number(request[REQ_COMMAND_REQUEST_DCC_COMMAND]));
                return(trameAnalysis);
            }
            else if (request[REQ_COMMAND_REQUEST_GET_AUTOMATION_LIST] == TRUE) {
                trameAnalysis.append(QString(" AUTLIST "));
                return(trameAnalysis);
            }
            else if (request[REQ_COMMAND_REQUEST_GET_DUMP] == TRUE) {
                trameAnalysis.append(QString(" DUMP "));
                return(trameAnalysis);
            }
            else if (request[REQ_COMMAND_REQUEST_GET_BOARD_STATUS] == TRUE) {
                trameAnalysis.append(QString(" BSTAT "));
                return(trameAnalysis);
            }
            else if (request[REQ_COMMAND_REQUEST_GET_GPIO_STATUS] == TRUE) {
                trameAnalysis.append(QString(" GSTAT "));
                return(trameAnalysis);
            }
            else if (request[REQ_COMMAND_REQUEST_GET_LPO_STATUS] == TRUE) {
                trameAnalysis.append(QString(" LSTAT "));
                return(trameAnalysis);
            }
            else if (request[REQ_COMMAND_REQUEST_GET_TRACK_STATUS] == TRUE) {
                trameAnalysis.append(QString(" LTSTAT "));
                return(trameAnalysis);
            }
            break;
        }
    }
    return(trameAnalysis);
}
void ScribeMainWindow::innoMakerDataReceived(unsigned char c)
{

    CANinput.gl_inputBuffer[CANinput.gl_InputBufferPointer]=c;
    CANinput.gl_InputBufferPointer++;
    if(CANinput.gl_InputBufferPointer>=MAXTRAMESIZE)CANinput.gl_InputBufferPointer-=MAXTRAMESIZE;
    int mode;
    if (CANinput.getInputRequestFromCAN(CANinput.gl_request,&mode)==TRUE) {
        QString line;
        if (mode==CAN_PRINT && !serial) {
            line=QString::fromUtf8(reinterpret_cast<const char*>(CANinput.gl_request));
        }
        else if (mode==CAN_REQUEST) {
            line = analyseCANinput(CANinput.gl_request);
        }

        // Affiche les trames CAN en couleur
        QTextCharFormat format;
        if (line.contains("BOARD 31 ")) format.setForeground(QBrush(Qt::blue));
        if (line.contains("BOARD 0 ")) format.setForeground(QBrush(Qt::magenta));
        if (line.contains("BOARD 1 ")) format.setForeground(QBrush(Qt::green));
        if (line.contains("BOARD 2 ")) format.setForeground(QBrush(QColor("#FFA500")));
        if (line.contains("BOARD 3 ")) format.setForeground(QBrush(Qt::red));

        QTextCursor cursor(CommandResult->document());
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("\n[CAN] " + line, format);
        CommandResult->ensureCursorVisible();
        CommandResult->repaint();


        // Init
        unsigned char dataCounter;
        for (dataCounter=0;dataCounter<REQUESTSIZE;dataCounter++)CANinput.gl_request[dataCounter]=0;
    }
}
void ScribeMainWindow::on_actionCheck_Program_triggered(){
    editor->getContent();
    emit checkTextSignal();
}

void ScribeMainWindow::on_actionUpdate_triggered() {
    if (!serial || !serial->isOpen()) {
        QMessageBox::critical(this, "Error", "Select a valid COM port!");
        return;
    }

    QStringList linesToSend;
    QString documentContents = editor->getContent();
    QTextStream stream(&documentContents, QIODevice::ReadOnly);
    QString line;
    while (stream.readLineInto(&line)) {
        line = line.trimmed();
        if (!line.startsWith("//") && !line.isEmpty()) {
            linesToSend.append(line + "\r");
        }
    }

    sendQueue = linesToSend;
    ui->sendProgressBar->setValue(0);
    ui->sendProgressBar->setMaximum(sendQueue.size());
    ui->sendProgressBar->setVisible(true);
    transmissionStoppedManually = false;
    processSendQueue();
}

void ScribeMainWindow::processSendQueue() {
    if (sendQueue.isEmpty()) {
        ui->sendProgressBar->setVisible(false);

        QTextCharFormat format;
        format.setForeground(QBrush(Qt::black));
        QTextCursor cursor(CommandResult->document());
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("[Terminé]", format);
        CommandResult->ensureCursorVisible();
        CommandResult->repaint();

        return;
    }

    QString line = sendQueue.takeFirst();
    QByteArray utf8Bytes = line.toUtf8();

    int total = ui->sendProgressBar->maximum();
    int sent = total - sendQueue.size();
    ui->sendProgressBar->setValue(sent);
    ui->sendProgressBar->setFormat(QString("%1 %").arg((100 * sent) / total));

    if (!serial) return;

    for (int i = 0; i < utf8Bytes.size(); ++i) {
        char ch = utf8Bytes.at(i);
        serial->write(&ch, 1);
        serial->waitForBytesWritten(m_waitTimeout);
        serial->waitForReadyRead(m_waitTimeout);

        if (transmissionStoppedManually) {

            QTextCharFormat format;
            format.setForeground(QBrush(Qt::black));
            QTextCursor cursor(CommandResult->document());
            cursor.movePosition(QTextCursor::End);
            cursor.insertText("[STOP]]", format);
            CommandResult->ensureCursorVisible();
            CommandResult->repaint();


            ui->sendProgressBar->setVisible(false);
            sendQueue.clear();
            transmissionStoppedManually = false;
            return;
        }
    }

    // Appelle la suite du traitement après retour à l'événement loop
    QTimer::singleShot(0, this, &ScribeMainWindow::processSendQueue);
}

void ScribeMainWindow::on_stopTransmissionButton_clicked() {
     transmissionStoppedManually = true;
}
void ScribeMainWindow::on_simpleCommand(){
    QString newLine = QChar('\r');
    newLine.append(simpleCommand->text()).append(QChar(' ')).append(QChar('\r'));
    QByteArray utf8Bytes = newLine.toUtf8();

    if (!serial) {
        QMessageBox::critical(this, "Error", "Select a port com first !");
        return;
    }

    for (char byte : utf8Bytes) {
        serial->write(&byte, 1);
        serial->waitForBytesWritten(m_waitTimeout);
        serial->waitForReadyRead(m_waitTimeout);

    }
}

/* Called when the user selects the Print option from the menu or toolbar (or uses Ctrl+P).
 * Allows the user to print the contents of the current document.
 */
void ScribeMainWindow::on_actionPrint_triggered()
{
    QPrinter printer;
    printer.setPrinterName(tr("Document printer"));
    QPrintDialog printDialog(&printer, this);

    if (printDialog.exec() != QPrintDialog::Rejected)
    {
        editor->print(&printer);
        ui->statusBar->showMessage("Printing", 2000);
    }
}


/* Called when the user tries to close a tab in the editor (or uses Ctrl+W). Allows the user
 * to save the contents of the tab if unsaved. Closes the tab, unless the file is unsaved
 * and the user declines saving. Returns true if the tab was closed and false otherwise.
 */
bool ScribeMainWindow::closeTab(Editor *tabToClose)
{
    Editor *currentTab = editor;
    bool closingCurrentTab = (tabToClose == currentTab);

    // Allow the user to see what tab they're closing if it's not the current one
    if (!closingCurrentTab)
    {
        tabbedEditor->setCurrentWidget(tabToClose);
    }

    // Don't close a tab immediately if it has unsaved contents
    if (tabToClose->isUnsaved())
    {
        QMessageBox::StandardButton selection = askUserToSave();

        if (selection == QMessageBox::StandardButton::Yes)
        {
            bool fileSaved = on_actionSaveTriggered();

            if (!fileSaved)
            {
                return false;
            }
        }

        else if (selection == QMessageBox::Cancel)
        {
            return false;
        }
    }

    int indexOfTabToClose = tabbedEditor->indexOf(tabToClose);
    tabbedEditor->removeTab(indexOfTabToClose);

    // If we closed the last tab, make a new one
    if (tabbedEditor->count() == 0)
    {
        on_actionNew_triggered();
    }

    // And finally, go back to original tab if the user was closing a different one
    if (!closingCurrentTab)
    {
        tabbedEditor->setCurrentWidget(currentTab);
    }

    return true;
}


/* Called when the user selects the Exit option from the menu. Allows the user
 * to save any unsaved files before quitting.
 */
void ScribeMainWindow::on_actionExit_triggered()
{
    QVector<Editor*> unsavedTabs = tabbedEditor->unsavedTabs();

    for (Editor *tab : unsavedTabs)
    {
        bool userClosedTab = closeTab(tab);

        if (!userClosedTab)
        {
            return;
        }
    }

    writeSettings();
    QApplication::quit();
}


/* Saves the main application state/settings so they may be
 * restored the next time the application is started. See
 * readSettings and the constructor for more info.
 */
void ScribeMainWindow::writeSettings()
{
    settings->setValue(WINDOW_SIZE_KEY, size());
    settings->setValue(WINDOW_POSITION_KEY, pos());
    settings->setValue(WINDOW_STATUS_BAR, ui->statusBar->isVisible());
    settings->setValue(WINDOW_TOOL_BAR, ui->mainToolBar->isVisible());
}


/* Reads the stored app settings and restores them.
 */
void ScribeMainWindow::readSettings()
{
    settings->apply(settings->value(WINDOW_SIZE_KEY, QSize(400, 400)),
                    [=](QVariant setting){ this->resize(setting.toSize()); });

    settings->apply(settings->value(WINDOW_POSITION_KEY, QPoint(200, 200)),
                    [=](QVariant setting){ this->move(setting.toPoint()); });

    settings->apply(settings->value(WINDOW_STATUS_BAR),
                    [=](QVariant setting) {
                        this->ui->statusBar->setVisible(qvariant_cast<bool>(setting));
                        this->ui->actionStatus_Bar->setChecked(qvariant_cast<bool>(setting));
                    });

    settings->apply(settings->value(WINDOW_TOOL_BAR),
                    [=](QVariant setting) {
                        this->ui->mainToolBar->setVisible(qvariant_cast<bool>(setting));
                        this->ui->actionTool_Bar->setChecked(qvariant_cast<bool>(setting));
                    });
}

/* Open Serial Link
 */

void ScribeMainWindow::openSerial() {
    if(!serial) {
        serial=new QSerialPort();
        connect(serial, &QSerialPort::readyRead, this, &ScribeMainWindow::onDataReceived);
    }
    serial->setPortName(gl_currentComPort);
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (!serial->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, "Error", "Can't open " + gl_currentComPort);
        delete serial;
        serial=Q_NULLPTR;
    }
}

void ScribeMainWindow::onDataReceived()
{
    if(!serial) return;
    while (serial->bytesAvailable()) {
        char c;
        qint64 bytesRead = serial->read(&c, 1);
        if (bytesRead > 0) {

            QTextCharFormat format;
            format.setForeground(QBrush(Qt::black));
            QTextCursor cursor(CommandResult->document());
            cursor.movePosition(QTextCursor::End);
            if (QChar(c).isPrint() || c=='\n') cursor.insertText(QString(c), format);
            CommandResult->ensureCursorVisible();
            CommandResult->repaint();
        }
    }
}

/* Called when the Undo operation is toggled by the editor.
 */
void ScribeMainWindow::toggleUndo(bool undoAvailable)
{
    ui->actionUndo->setEnabled(undoAvailable);
}


/* Called when the Redo operation is toggled by the editor.
 */
void ScribeMainWindow::toggleRedo(bool redoAvailable)
{
    ui->actionRedo->setEnabled(redoAvailable);
}


/* Called when the user performs the Undo operation.
 */
void ScribeMainWindow::on_actionUndo_triggered()
{
    if (ui->actionUndo->isEnabled())
    {
        editor->undo();
    }
}


/* Called when the user performs the Redo operation.
 */
void ScribeMainWindow::on_actionRedo_triggered()
{
    if (ui->actionRedo->isEnabled())
    {
        editor->redo();
    }
}


/* Called when the Copy and Cut operations are toggled by the editor.
 */
void ScribeMainWindow::toggleCopyAndCut(bool copyCutAvailable)
{
    ui->actionCopy->setEnabled(copyCutAvailable);
    ui->actionCut->setEnabled(copyCutAvailable);
}


/* Called when the user performs the Cut operation.
 */
void ScribeMainWindow::on_actionCut_triggered()
{
    if (ui->actionCut->isEnabled())
    {
        editor->cut();
    }
}


/* Called when the user performs the Copy operation.
 */
void ScribeMainWindow::on_actionCopy_triggered()
{
    if (ui->actionCopy->isEnabled())
    {
        editor->copy();
    }
}


/* Called when the user performs the Paste operation.
 */
void ScribeMainWindow::on_actionPaste_triggered()
{
    editor->paste();
}


/* Called when the user explicitly selects the Find option from the menu
 * (or uses Ctrl+F). Launches a dialog that prompts the user to enter a search query.
 */
void ScribeMainWindow::on_actionFind_triggered()
{
    launchFindDialog();
}


/* Called when the user explicitly selects the Go To option from the menu (or uses Ctrl+G).
 * Launches a Go To dialog that prompts the user to enter a line number they wish to jump to.
 */
void ScribeMainWindow::on_actionGo_To_triggered()
{
    launchGotoDialog();
}


/* Called when the user explicitly selects the Select All option from the menu (or uses Ctrl+A).
 */
void ScribeMainWindow::on_actionSelect_All_triggered()
{
    editor->selectAll();
}


/* Called when the user explicitly selects the Time/Date option from the menu (or uses F5).
 */
void ScribeMainWindow::on_actionTime_Date_triggered()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    editor->insertPlainText(currentTime.toString());
}


/* Called when the user selects the Font option from the menu. Launches a font selection dialog.
 */
void ScribeMainWindow::on_actionFont_triggered()
{
    tabbedEditor->promptFontSelection();
}


/* Called when the user selects the Auto Indent option from the Format menu.
 */
void ScribeMainWindow::on_actionAuto_Indent_triggered()
{
    bool shouldAutoIndent = ui->actionAuto_Indent->isChecked();
    bool autoIndentToggled = tabbedEditor->applyAutoIndentation(shouldAutoIndent);

    // If the user canceled the operation, reverse the checking
    if (!autoIndentToggled)
    {
        ui->actionAuto_Indent->setChecked(!shouldAutoIndent);
    }
}


/* Called when the user selects the Word Wrap option from the Format menu.
 */
void ScribeMainWindow::on_actionWord_Wrap_triggered()
{
    tabbedEditor->applyWordWrapping(ui->actionWord_Wrap->isChecked());
}


/* Toggles the visibility of the given widget. It is assumed that this
 * widget is part of the main window. Otherwise, the effect may not be seen.
 */
void ScribeMainWindow::toggleVisibilityOf(QWidget *widget)
{
    bool opposite = !widget->isVisible();
    widget->setVisible(opposite);
}


/* Toggles the visibility of the status bar.
 */
void ScribeMainWindow::on_actionStatus_Bar_triggered()
{
    toggleVisibilityOf(ui->statusBar);
}


/* Toggles the visibility of the main tool bar
*/
void ScribeMainWindow::on_actionTool_Bar_triggered()
{
    toggleVisibilityOf(ui->mainToolBar);
}


/* Overrides the QWidget closeEvent virtual method. Called when the user tries
 * to close the main application window conventually via the red X. Allows the
 * user to save any unsaved files before quitting.
 */
void ScribeMainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    on_actionExit_triggered();
}

