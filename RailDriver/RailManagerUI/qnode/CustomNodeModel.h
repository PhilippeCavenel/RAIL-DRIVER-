#ifndef CUSTOMNODEMODEL_HPP
#define CUSTOMNODEMODEL_HPP

#include <QtNodes/NodeDelegateModel>
using namespace QtNodes;

class CustomNodeModel : public NodeDelegateModel {
public:
    CustomNodeModel() = default;

    QString caption() const override { return _caption; }
    bool captionVisible() const override { return true; }

    unsigned int nPorts(PortType portType) const override {
        return (portType == PortType::In || portType == PortType::Out) ? 1 : 0;
    }

    QString portCaption(PortType portType, PortIndex index) const override {
        return (portType == PortType::In) ? _inputCaption : _outputCaption;
    }

    bool portCaptionVisible(PortType, PortIndex) const override { return true; }

    QWidget *embeddedWidget() override { return nullptr; }

    void setCaption(const QString &caption) { _caption = caption; }
    void setInputCaption(const QString &caption) { _inputCaption = caption; }
    void setOutputCaption(const QString &caption) { _outputCaption = caption; }

private:
    QString _caption = "Node";
    QString _inputCaption = "In";
    QString _outputCaption = "Out";
};

#endif // CUSTOMNODEMODEL_HPP
