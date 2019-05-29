/********************************************************************************
** Form generated from reading UI file 'Garganture.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GARGANTURE_H
#define UI_GARGANTURE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GargantureClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *GargantureClass)
    {
        if (GargantureClass->objectName().isEmpty())
            GargantureClass->setObjectName(QString::fromUtf8("GargantureClass"));
        GargantureClass->resize(600, 400);
        menuBar = new QMenuBar(GargantureClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        GargantureClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(GargantureClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        GargantureClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(GargantureClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        GargantureClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(GargantureClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        GargantureClass->setStatusBar(statusBar);

        retranslateUi(GargantureClass);

        QMetaObject::connectSlotsByName(GargantureClass);
    } // setupUi

    void retranslateUi(QMainWindow *GargantureClass)
    {
        GargantureClass->setWindowTitle(QApplication::translate("GargantureClass", "Garganture", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GargantureClass: public Ui_GargantureClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GARGANTURE_H
