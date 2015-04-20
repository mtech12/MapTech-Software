/********************************************************************************
** Form generated from reading UI file 'mt500.ui'
**
** Created: Thu Oct 10 14:43:12 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MT500_H
#define UI_MT500_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MT500
{
public:
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QWidget *DataTab;
    QTextBrowser *DataBrowser;
    QPushButton *clrButton;
    QWidget *tab;
    QTextBrowser *CloudBrowser;
    QWidget *StatusTab;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *gidLabel;
    QLabel *inCOMLabel;
    QLabel *outCOMLabel;
    QLabel *rxLabel;
    QFrame *frame_4;
    QPushButton *reconfigureButton;
    QLabel *updateLbl;
    QLabel *label_15;
    QComboBox *runModeComboBox;
    QWidget *PortsTab;
    QTableWidget *ipTable;
    QWidget *SetupTab;
    QLabel *label_8;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label_9;
    QLineEdit *ipLineEdit;
    QLabel *label_10;
    QLineEdit *portLineEdit;
    QLabel *label_11;
    QComboBox *dataSelect;
    QFrame *frame;
    QPushButton *addButton;
    QLabel *addLabel;
    QFrame *frame_2;
    QLabel *addLabel_2;
    QLabel *label_12;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_13;
    QComboBox *delSelect;
    QPushButton *delButton;
    QLabel *delLabel;
    QFrame *frame_3;
    QLabel *label_14;
    QTextBrowser *testBrowser;
    QPushButton *testButton;
    QWidget *AboutTab;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *versionLbl;

    void setupUi(QMainWindow *MT500)
    {
        if (MT500->objectName().isEmpty())
            MT500->setObjectName(QString::fromUtf8("MT500"));
        MT500->resize(600, 400);
        centralWidget = new QWidget(MT500);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(0, 0, 641, 401));
        DataTab = new QWidget();
        DataTab->setObjectName(QString::fromUtf8("DataTab"));
        DataBrowser = new QTextBrowser(DataTab);
        DataBrowser->setObjectName(QString::fromUtf8("DataBrowser"));
        DataBrowser->setGeometry(QRect(10, 20, 551, 301));
        clrButton = new QPushButton(DataTab);
        clrButton->setObjectName(QString::fromUtf8("clrButton"));
        clrButton->setGeometry(QRect(450, 330, 98, 27));
        tabWidget->addTab(DataTab, QString());
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        CloudBrowser = new QTextBrowser(tab);
        CloudBrowser->setObjectName(QString::fromUtf8("CloudBrowser"));
        CloudBrowser->setGeometry(QRect(10, 20, 551, 301));
        tabWidget->addTab(tab, QString());
        StatusTab = new QWidget();
        StatusTab->setObjectName(QString::fromUtf8("StatusTab"));
        verticalLayoutWidget = new QWidget(StatusTab);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(20, 20, 551, 141));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        gidLabel = new QLabel(verticalLayoutWidget);
        gidLabel->setObjectName(QString::fromUtf8("gidLabel"));

        verticalLayout->addWidget(gidLabel);

        inCOMLabel = new QLabel(verticalLayoutWidget);
        inCOMLabel->setObjectName(QString::fromUtf8("inCOMLabel"));

        verticalLayout->addWidget(inCOMLabel);

        outCOMLabel = new QLabel(verticalLayoutWidget);
        outCOMLabel->setObjectName(QString::fromUtf8("outCOMLabel"));

        verticalLayout->addWidget(outCOMLabel);

        rxLabel = new QLabel(verticalLayoutWidget);
        rxLabel->setObjectName(QString::fromUtf8("rxLabel"));

        verticalLayout->addWidget(rxLabel);

        frame_4 = new QFrame(StatusTab);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setGeometry(QRect(10, 10, 571, 231));
        frame_4->setFrameShape(QFrame::StyledPanel);
        frame_4->setFrameShadow(QFrame::Raised);
        reconfigureButton = new QPushButton(frame_4);
        reconfigureButton->setObjectName(QString::fromUtf8("reconfigureButton"));
        reconfigureButton->setGeometry(QRect(10, 190, 161, 27));
        updateLbl = new QLabel(frame_4);
        updateLbl->setObjectName(QString::fromUtf8("updateLbl"));
        updateLbl->setGeometry(QRect(470, 163, 71, 20));
        label_15 = new QLabel(frame_4);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(10, 160, 91, 17));
        runModeComboBox = new QComboBox(frame_4);
        runModeComboBox->setObjectName(QString::fromUtf8("runModeComboBox"));
        runModeComboBox->setGeometry(QRect(90, 155, 78, 27));
        tabWidget->addTab(StatusTab, QString());
        frame_4->raise();
        verticalLayoutWidget->raise();
        PortsTab = new QWidget();
        PortsTab->setObjectName(QString::fromUtf8("PortsTab"));
        ipTable = new QTableWidget(PortsTab);
        if (ipTable->columnCount() < 3)
            ipTable->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        ipTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        ipTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        ipTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        if (ipTable->rowCount() < 6)
            ipTable->setRowCount(6);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        ipTable->setVerticalHeaderItem(0, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        ipTable->setVerticalHeaderItem(1, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        ipTable->setVerticalHeaderItem(2, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        ipTable->setVerticalHeaderItem(3, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        ipTable->setVerticalHeaderItem(4, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        ipTable->setVerticalHeaderItem(5, __qtablewidgetitem8);
        ipTable->setObjectName(QString::fromUtf8("ipTable"));
        ipTable->setGeometry(QRect(20, 20, 431, 211));
        ipTable->setMinimumSize(QSize(411, 0));
        tabWidget->addTab(PortsTab, QString());
        SetupTab = new QWidget();
        SetupTab->setObjectName(QString::fromUtf8("SetupTab"));
        label_8 = new QLabel(SetupTab);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(20, 20, 101, 17));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label_8->setFont(font);
        horizontalLayoutWidget = new QWidget(SetupTab);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(20, 40, 561, 31));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label_9 = new QLabel(horizontalLayoutWidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        horizontalLayout->addWidget(label_9);

        ipLineEdit = new QLineEdit(horizontalLayoutWidget);
        ipLineEdit->setObjectName(QString::fromUtf8("ipLineEdit"));
        ipLineEdit->setMinimumSize(QSize(120, 0));
        ipLineEdit->setMaximumSize(QSize(120, 16777215));

        horizontalLayout->addWidget(ipLineEdit);

        label_10 = new QLabel(horizontalLayoutWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        horizontalLayout->addWidget(label_10);

        portLineEdit = new QLineEdit(horizontalLayoutWidget);
        portLineEdit->setObjectName(QString::fromUtf8("portLineEdit"));
        portLineEdit->setMaximumSize(QSize(60, 16777215));

        horizontalLayout->addWidget(portLineEdit);

        label_11 = new QLabel(horizontalLayoutWidget);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout->addWidget(label_11);

        dataSelect = new QComboBox(horizontalLayoutWidget);
        dataSelect->setObjectName(QString::fromUtf8("dataSelect"));
        dataSelect->setMinimumSize(QSize(125, 0));

        horizontalLayout->addWidget(dataSelect);

        frame = new QFrame(SetupTab);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(10, 10, 581, 111));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        addButton = new QPushButton(frame);
        addButton->setObjectName(QString::fromUtf8("addButton"));
        addButton->setGeometry(QRect(460, 70, 98, 27));
        addLabel = new QLabel(frame);
        addLabel->setObjectName(QString::fromUtf8("addLabel"));
        addLabel->setGeometry(QRect(10, 70, 111, 17));
        frame_2 = new QFrame(SetupTab);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setGeometry(QRect(10, 130, 581, 91));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        addLabel_2 = new QLabel(frame_2);
        addLabel_2->setObjectName(QString::fromUtf8("addLabel_2"));
        addLabel_2->setGeometry(QRect(10, 70, 111, 17));
        label_12 = new QLabel(frame_2);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(10, 10, 101, 17));
        label_12->setFont(font);
        horizontalLayoutWidget_2 = new QWidget(frame_2);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(10, 30, 391, 41));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_13 = new QLabel(horizontalLayoutWidget_2);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        horizontalLayout_2->addWidget(label_13);

        delSelect = new QComboBox(horizontalLayoutWidget_2);
        delSelect->setObjectName(QString::fromUtf8("delSelect"));
        delSelect->setMinimumSize(QSize(125, 0));

        horizontalLayout_2->addWidget(delSelect);

        delButton = new QPushButton(horizontalLayoutWidget_2);
        delButton->setObjectName(QString::fromUtf8("delButton"));

        horizontalLayout_2->addWidget(delButton);

        delLabel = new QLabel(horizontalLayoutWidget_2);
        delLabel->setObjectName(QString::fromUtf8("delLabel"));

        horizontalLayout_2->addWidget(delLabel);

        frame_3 = new QFrame(SetupTab);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setGeometry(QRect(10, 230, 581, 131));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        label_14 = new QLabel(frame_3);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(10, 10, 121, 17));
        label_14->setFont(font);
        testBrowser = new QTextBrowser(frame_3);
        testBrowser->setObjectName(QString::fromUtf8("testBrowser"));
        testBrowser->setGeometry(QRect(10, 40, 561, 81));
        testButton = new QPushButton(frame_3);
        testButton->setObjectName(QString::fromUtf8("testButton"));
        testButton->setGeometry(QRect(460, 10, 86, 27));
        tabWidget->addTab(SetupTab, QString());
        frame_2->raise();
        frame->raise();
        label_8->raise();
        horizontalLayoutWidget->raise();
        frame_3->raise();
        AboutTab = new QWidget();
        AboutTab->setObjectName(QString::fromUtf8("AboutTab"));
        label = new QLabel(AboutTab);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(15, 30, 551, 31));
        QFont font1;
        font1.setPointSize(17);
        label->setFont(font1);
        label_2 = new QLabel(AboutTab);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(15, 60, 551, 31));
        label_2->setFont(font1);
        label_3 = new QLabel(AboutTab);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(15, 120, 161, 31));
        label_3->setFont(font1);
        label_4 = new QLabel(AboutTab);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(15, 150, 171, 31));
        label_4->setFont(font1);
        label_5 = new QLabel(AboutTab);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(15, 180, 251, 31));
        label_5->setFont(font1);
        label_6 = new QLabel(AboutTab);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(15, 210, 161, 31));
        label_6->setFont(font1);
        label_7 = new QLabel(AboutTab);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(15, 240, 261, 31));
        label_7->setFont(font1);
        versionLbl = new QLabel(AboutTab);
        versionLbl->setObjectName(QString::fromUtf8("versionLbl"));
        versionLbl->setGeometry(QRect(15, 290, 351, 31));
        versionLbl->setFont(font1);
        tabWidget->addTab(AboutTab, QString());
        MT500->setCentralWidget(centralWidget);

        retranslateUi(MT500);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MT500);
    } // setupUi

    void retranslateUi(QMainWindow *MT500)
    {
        MT500->setWindowTitle(QApplication::translate("MT500", "MT500", 0, QApplication::UnicodeUTF8));
        clrButton->setText(QApplication::translate("MT500", "Clear", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(DataTab), QApplication::translate("MT500", "Data", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MT500", "Cloud", 0, QApplication::UnicodeUTF8));
        gidLabel->setText(QApplication::translate("MT500", "Locality GID:", 0, QApplication::UnicodeUTF8));
        inCOMLabel->setText(QApplication::translate("MT500", "Incoming COM Port:", 0, QApplication::UnicodeUTF8));
        outCOMLabel->setText(QApplication::translate("MT500", "Outgoing COM Port:", 0, QApplication::UnicodeUTF8));
        rxLabel->setText(QApplication::translate("MT500", "Messages Received:", 0, QApplication::UnicodeUTF8));
        reconfigureButton->setText(QApplication::translate("MT500", "Update Configuration", 0, QApplication::UnicodeUTF8));
        updateLbl->setText(QString());
        label_15->setText(QApplication::translate("MT500", "Run Mode:", 0, QApplication::UnicodeUTF8));
        runModeComboBox->clear();
        runModeComboBox->insertItems(0, QStringList()
         << QApplication::translate("MT500", "Debug", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MT500", "Normal", 0, QApplication::UnicodeUTF8)
        );
        tabWidget->setTabText(tabWidget->indexOf(StatusTab), QApplication::translate("MT500", "Status", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = ipTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("MT500", "IP Address", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = ipTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("MT500", "Port", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = ipTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("MT500", "Data Type", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = ipTable->verticalHeaderItem(0);
        ___qtablewidgetitem3->setText(QApplication::translate("MT500", "Destination 1", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = ipTable->verticalHeaderItem(1);
        ___qtablewidgetitem4->setText(QApplication::translate("MT500", "Destination 2", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem5 = ipTable->verticalHeaderItem(2);
        ___qtablewidgetitem5->setText(QApplication::translate("MT500", "Destination 3", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem6 = ipTable->verticalHeaderItem(3);
        ___qtablewidgetitem6->setText(QApplication::translate("MT500", "Destination 4", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem7 = ipTable->verticalHeaderItem(4);
        ___qtablewidgetitem7->setText(QApplication::translate("MT500", "Destination 5", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem8 = ipTable->verticalHeaderItem(5);
        ___qtablewidgetitem8->setText(QApplication::translate("MT500", "Destination 6", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(PortsTab), QApplication::translate("MT500", "Ports", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("MT500", "Add a Port", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("MT500", "IP Address:", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("MT500", "Port:", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("MT500", "Data Type:", 0, QApplication::UnicodeUTF8));
        dataSelect->clear();
        dataSelect->insertItems(0, QStringList()
         << QApplication::translate("MT500", "Raw", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MT500", "Processed", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MT500", "Base Station", 0, QApplication::UnicodeUTF8)
        );
        addButton->setText(QApplication::translate("MT500", "Add", 0, QApplication::UnicodeUTF8));
        addLabel->setText(QString());
        addLabel_2->setText(QString());
        label_12->setText(QApplication::translate("MT500", "Delete a Port", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("MT500", "IP Address:", 0, QApplication::UnicodeUTF8));
        delButton->setText(QApplication::translate("MT500", "Delete", 0, QApplication::UnicodeUTF8));
        delLabel->setText(QString());
        label_14->setText(QApplication::translate("MT500", "Test Connection", 0, QApplication::UnicodeUTF8));
        testButton->setText(QApplication::translate("MT500", "Test", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(SetupTab), QApplication::translate("MT500", "Setup", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MT500", "MT500 is a product of MT Products Division of", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MT500", "MapTech, Inc. All Rights Reserved.", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MT500", "MapTech, Inc.", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MT500", "3154 State St.", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("MT500", "Blacksburg, VA 24060", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("MT500", "540-961-7864", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("MT500", "www.maptech-inc.com", 0, QApplication::UnicodeUTF8));
        versionLbl->setText(QApplication::translate("MT500", "Program Version: V.007", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(AboutTab), QApplication::translate("MT500", "About", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MT500: public Ui_MT500 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MT500_H
