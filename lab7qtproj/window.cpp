//-------------------------------------------------------------------------------------------
//  This is a demo. program for open gl vertex arrays
//-------------------------------------------------------------------------------------------
#include <QtGui>
#include <QtOpenGL>
#include "window.h"

#define BIT0 0x00000001   // alpha
#define BIT1 0x00000002   // no buttons
#define BIT2 0x00000004   // set don't use native dialog

//------------------------------------------------------------------------------------
// Creates and initializes the main window for application
//------------------------------------------------------------------------------------
Window::Window(QWidget *parent):QDialog(parent)
{
    //We create an instance of GLWidget component we built in glwidget.h
	m_glWidget = new GLWidget;
	
	//Setup application interface. Creates all the required components and sliders.
	setupUi(this);
    //We need to attach our m_glWidget to glWidgetArea
    //All our drawings will be on glWidgetArea
    glWidgetArea->setWidget(m_glWidget);


}

void Window::openImage()
{
    QString fileBuf;

    qDebug() << " opening image\n";
    fileBuf = QFileDialog::getOpenFileName(this,  "Open Image", "~/", "Image Files (*.png *.jpg)" );
    if (fileBuf=="") return;   // cancelled
   // QMessageBox::information( this, "Qtglskel: Opened image file ", fileBuf, QMessageBox::Ok );

    qDebug()<< "calling open in glwidget "<< fileBuf;
    m_glWidget->openImage(fileBuf);
}

void Window::saveImage()
{
    QString fileBuf;

    qDebug() << " saving image\n";
    fileBuf = QFileDialog::getSaveFileName(this, tr("Save File"),
                               "/home/untitled.png",
                               tr("Images (*.png *.xpm *.jpg)"));
     if (fileBuf=="") return;   // cancelled
   // QMessageBox::information( this, "Qtglskel: Opened image file ", fileBuf, QMessageBox::Ok );
    qDebug()<< "calling save in glwidget "<< fileBuf;
    m_glWidget->saveImage(fileBuf);
}

void Window::makeImage()
{
    m_glWidget->makeImage();
}

void Window::on_openSceneButton_clicked()
{
    QString fileBuf;

    fileBuf = QFileDialog::getOpenFileName(this,  "SelectScene", "~/", "Text Files (*.txt *.scn)" );
    if (fileBuf=="") return;   // cancelled

    qDebug()<< "calling open in glwidget "<< fileBuf;
    m_glWidget->openScene(fileBuf);
}
