/***************************************************************************
                          MyWidgets.cpp  -  description
                             -------------------
    begin                : Mon Dec 11 2000
    copyright            : (C) 2000 by Rainer Lehrig
    email                : lehrig@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "pvdefine.h"
#include <stdio.h>
#include <ctype.h>
#include "opt.h"
#include "MyWidgets.h"
#include "interpreter.h"

#include "qtabbar.h"
#include "qpainter.h"
#include "qmessagebox.h"
#include <QPixmap>
#include <QMouseEvent>
#include <QWebHistory>
#include "tcputil.h"

extern OPT opt;

static const char *decode(QString text)
{
  if(opt.codec == pvbUTF8) return text.toUtf8();
  return text.toAscii();
}

////////////////////////////////////////////////////////////////////////////////
MyDialog::MyDialog(Interpreter *inter, int *sock, int ident, QWidget * parent, const char *name, bool modal)
         :QDialog(parent, Qt::Widget)
{
  setAttribute(Qt::WA_DeleteOnClose);
  s  = sock;
  id = ident;
  interpreter = inter;
  setModal(modal);
  if(name != NULL) setObjectName(name);
  QIcon appIcon(":/images/app.png");
  setWindowIcon(appIcon);
}

MyDialog::~MyDialog()
{
#ifndef PVDEVELOP
  interpreter->slotModalTerminate();
#endif
}

void MyDialog::done(int result)
{
#ifndef PVDEVELOP
  char buf[80];
  sprintf(buf,"QPushButton(-1)\n");
  tcp_send(s,buf,strlen(buf));
  //interpreter->slotModalTerminate();
#endif
  if(result == 0) return;
}

////////////////////////////////////////////////////////////////////////////////
MyQWidget::MyQWidget(int *sock, int ident, QWidget * parent, const char * name)
          :QWidget(parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
}

MyQWidget::~MyQWidget()
{
}

//void MyQWidget::mousePressEvent(QMouseEvent *e)
//{
//  if(e->button() == Qt::RightButton)
//  {
//    char buf[80];
//
//    sprintf(buf,"QMouseRight(%d)\n",id);
//    tcp_send(s,buf,strlen(buf));
//  }
//}

////////////////////////////////////////////////////////////////////////////////
MyLabel::MyLabel(int *sock, int ident, QWidget * parent, const char *name)
              :QLabel(parent)
{
  s = sock;
  id = ident;
  row = col = -1;
  if(name != NULL) setObjectName(name);
}

MyLabel::~MyLabel()
{
}

void MyLabel::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  if(row != -1 || col != -1)
  {
    sprintf(buf,"QTableValue(%d,%d,%d,\"%s\")\n",id,row,col,decode(text()));
    tcp_send(s,buf,strlen(buf));
  }
  else
  {
    sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
    tcp_send(s,buf,strlen(buf));
    QLabel::mousePressEvent(event);
  }  
}

void MyLabel::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  if(row != -1 || col != -1)
  {
  }
  else
  {
    sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
    if(underMouse()) tcp_send(s,buf,strlen(buf));
  }  
  QLabel::mouseReleaseEvent(event);
}

void MyLabel::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QLabel::enterEvent(event);
}

void MyLabel::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QLabel::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyQPushButton::MyQPushButton(int *sock, int ident, QWidget * parent, const char * name)
              :QPushButton(parent)
{
  s = sock;
  id = ident;
  row = col = -1;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(pressed()),  SLOT(slotPressed()));
  connect(this, SIGNAL(released()), SLOT(slotReleased()));
  connect(this, SIGNAL(clicked()),  SLOT(slotClicked()));
}

MyQPushButton::~MyQPushButton()
{
}

void MyQPushButton::slotClicked()
{
char buf[80];

  if(row != -1 || col != -1)
  {
    sprintf(buf,"QTableValue(%d,%d,%d,\"%s\")\n",id,row,col,decode(text()));
    tcp_send(s,buf,strlen(buf));
  }
  else
  {
    sprintf(buf,"QPushButton(%d)\n",id);
    tcp_send(s,buf,strlen(buf));
  }  
}

void MyQPushButton::slotPressed()
{
char buf[80];

  if(row != -1 || col != -1) return;
  sprintf(buf,"QPushButtonPressed(%d)\n",id);
  tcp_send(s,buf,strlen(buf));
}

void MyQPushButton::slotReleased()
{
char buf[80];

  if(row != -1 || col != -1) return;
  sprintf(buf,"QPushButtonReleased(%d)\n",id);
  tcp_send(s,buf,strlen(buf));
}

void MyQPushButton::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QPushButton::enterEvent(event);
}

void MyQPushButton::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QPushButton::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyLineEdit::MyLineEdit(int *sock, int ident, QWidget * parent, const char * name)
           :QLineEdit(parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(textChanged(const QString &)), SLOT(slotTextChanged(const QString &)));
  connect(this, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
}

MyLineEdit::~MyLineEdit()
{
}

void MyLineEdit::slotTextChanged(const QString &txt)
{
  char buf[MAX_PRINTF_LENGTH];

  if(txt.length()+40 > MAX_PRINTF_LENGTH) return;
  sprintf(buf,"text(%d,\"%s\")\n", id, decode(txt));
  tcp_send(s,buf,strlen(buf));
}

void MyLineEdit::slotReturnPressed()
{
  char buf[MAX_PRINTF_LENGTH];

  sprintf(buf,"QPushButton(%d,\"%s\")\n",id, decode(text()));
  tcp_send(s,buf,strlen(buf));
}

void MyLineEdit::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QLineEdit::mousePressEvent(event);
}

void MyLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QLineEdit::mouseReleaseEvent(event);
}

void MyLineEdit::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QLineEdit::enterEvent(event);
}

void MyLineEdit::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QLineEdit::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyComboBox::MyComboBox(int *sock, int ident, QWidget * parent, const char * name)
           :QComboBox(parent)
{
  s = sock;
  id = ident;
  row = col = -1;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(activated(const QString &)), SLOT(slotActivated(const QString &)));
}

MyComboBox::~MyComboBox()
{
}

void MyComboBox::slotActivated(const QString &txt)
{
  char buf[MAX_PRINTF_LENGTH];
  QString txt2 = txt;
  
  if(txt2.isEmpty())
  {
    txt2.sprintf("index%d", currentIndex());
  }

  if(row != -1 || col != -1)
  {
    sprintf(buf,"QTableValue(%d,%d,%d,\"%s\")\n",id,row,col,decode(txt2));
    tcp_send(s,buf,strlen(buf));
  }
  else
  {
    if(txt2.length()+40 > MAX_PRINTF_LENGTH) return;
    sprintf(buf,"text(%d,\"%s\")\n", id, decode(txt2));
    tcp_send(s,buf,strlen(buf));
  }
}

void MyComboBox::removeItemByName(QString name)
{
  int i = 0;
  while(i < count())
  {
    if(itemText(i) == name) removeItem(i);
    i++;
  }
}

void MyComboBox::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QComboBox::mousePressEvent(event);
}

void MyComboBox::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QComboBox::mouseReleaseEvent(event);
}

void MyComboBox::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QComboBox::enterEvent(event);
}

void MyComboBox::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QComboBox::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MySlider::MySlider(int *sock, int ident, int minValue, int maxValue, int pageStep, int value, Qt::Orientation orientation, QWidget * parent, const char * name)
         :QSlider(orientation,parent)
{
  s = sock;
  id = ident;
  setMaximum(maxValue);
  setMinimum(minValue);
  setPageStep(pageStep);
  setValue(value);
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)));
}

MySlider::~MySlider()
{
}

void MySlider::slotValueChanged(int value)
{
  char buf[80];

  sprintf(buf,"slider(%d,%d)\n",id,value);
  tcp_send(s,buf,strlen(buf));
}

void MySlider::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QSlider::mousePressEvent(event);
}

void MySlider::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QSlider::mouseReleaseEvent(event);
}

void MySlider::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QSlider::enterEvent(event);
}

void MySlider::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QSlider::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyCheckBox::MyCheckBox(int *sock, int ident, QWidget * parent, const char * name)
           :QCheckBox(parent)
{
  s = sock;
  id = ident;
  row = col = -1;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(clicked()), SLOT(slotClicked()));
}

MyCheckBox::~MyCheckBox()
{
}

void MyCheckBox::slotClicked()
{
char buf[80];

  if(row != -1 || col != -1)
  {
    QString txt;
    QString sbuf = text();
    if(isChecked()) txt = "1," + sbuf;
    else            txt = "0," + sbuf;
    sprintf(buf,"QTableValue(%d,%d,%d,\"%s\")\n",id,row,col,decode(txt));
    tcp_send(s,buf,strlen(buf));
  }
  else
  {
    if(isChecked()) sprintf(buf,"check(%d,1)\n",id);
    else            sprintf(buf,"check(%d,0)\n",id);
    tcp_send(s,buf,strlen(buf));
  }
}

void MyCheckBox::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QCheckBox::mousePressEvent(event);
}

void MyCheckBox::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QCheckBox::mouseReleaseEvent(event);
}

void MyCheckBox::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QCheckBox::enterEvent(event);
}

void MyCheckBox::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QCheckBox::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyRadioButton::MyRadioButton(int *sock, int ident, QWidget * parent, const char * name)
              :QRadioButton(parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));
}

MyRadioButton::~MyRadioButton()
{
}

void MyRadioButton::slotToggled(bool on)
{
char buf[80];

  if(on) sprintf(buf,"radio(%d,1)\n",id);
  else   sprintf(buf,"radio(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
}

void MyRadioButton::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QRadioButton::mousePressEvent(event);
}

void MyRadioButton::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QRadioButton::mouseReleaseEvent(event);
}

void MyRadioButton::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QRadioButton::enterEvent(event);
}

void MyRadioButton::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QRadioButton::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyButtonGroup::MyButtonGroup(int *sock, int ident, int columns, Qt::Orientation o, QString title, QWidget * parent, const char * name)
              :QGroupBox(title,parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  if(columns == (int) o) return; // troll porting murx
}

MyButtonGroup::~MyButtonGroup()
{
}

void MyButtonGroup::mousePressEvent(QMouseEvent *e)
{
  char buf[80];

  if(e->button() == Qt::RightButton)
  {
    sprintf(buf,"QMouseRight(%d)\n",id);
    tcp_send(s,buf,strlen(buf));
  }
  else
  {
    sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, e->x(), e->y());
    tcp_send(s,buf,strlen(buf));
  }
  //QGroupBox::mousePressEvent(event);
}

void MyButtonGroup::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  //QGroupBox::mouseReleaseEvent(event);
}

void MyButtonGroup::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QGroupBox::enterEvent(event);
}

void MyButtonGroup::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QGroupBox::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyFrame::MyFrame(int *sock, int ident, int shape, int shadow, int line_width, int margin,
                 QWidget * parent, const char * name, Qt::WFlags f)
        :QFrame(parent, f)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  setFrameShape  ((QFrame::Shape)  shape);
  setFrameShadow ((QFrame::Shadow) shadow);
  setLineWidth   (line_width);
  //setMargin      (margin);
  if(margin == -1000) return;
}

MyFrame::~MyFrame()
{
}

void MyFrame::mousePressEvent(QMouseEvent *e)
{
  char buf[80];

  if(e->button() == Qt::RightButton)
  {
    sprintf(buf,"QMouseRight(%d)\n",id);
    tcp_send(s,buf,strlen(buf));
  }
  else
  {
    sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, e->x(), e->y());
    tcp_send(s,buf,strlen(buf));
  }
  //QFrame::mousePressEvent(event);
}

void MyFrame::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  //QFrame::mouseReleaseEvent(event);
}

void MyFrame::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QFrame::enterEvent(event);
}

void MyFrame::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QFrame::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyQTabWidget::MyQTabWidget(int *sock, int ident, QWidget *parent, const char *name, Qt::WFlags f)
             :QTabWidget(parent)
{
  id = ident;
  s  = sock;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(currentChanged(int)), SLOT(slotCurrentChanged(int)));
  if(f==0) return;
}

MyQTabWidget::~MyQTabWidget()
{
}

void MyQTabWidget::slotCurrentChanged(int index)
{
char buf[80];

  sprintf(buf,"tab(%d,%d)\n",id,index);
  tcp_send(s,buf,strlen(buf));
}

void MyQTabWidget::mousePressEvent(QMouseEvent *e)
{
  char buf[80];

  if(e->button() == Qt::RightButton)
  {
    sprintf(buf,"QMouseRight(%d)\n",id);
    tcp_send(s,buf,strlen(buf));
  }
  else
  {
    sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, e->x(), e->y());
    tcp_send(s,buf,strlen(buf));
  }
  //QTabWidget::mousePressEvent(event);
}

void MyQTabWidget::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QTabWidget::mouseReleaseEvent(event);
}

void MyQTabWidget::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QTabWidget::enterEvent(event);
}

void MyQTabWidget::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QTabWidget::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyQToolBox::MyQToolBox(int *sock, int ident, QWidget *parent, const char *name, Qt::WFlags f)
             :QToolBox(parent,f)
{
  id = ident;
  s  = sock;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(currentChanged(int)), SLOT(slotCurrentChanged(int)));
}

MyQToolBox::~MyQToolBox()
{
}

void MyQToolBox::slotCurrentChanged(int index)
{
char buf[80];

  sprintf(buf,"tab(%d,%d)\n",id,index);
  tcp_send(s,buf,strlen(buf));
}

void MyQToolBox::mousePressEvent(QMouseEvent *e)
{
  char buf[80];

  if(e->button() == Qt::RightButton)
  {
    sprintf(buf,"QMouseRight(%d)\n",id);
    tcp_send(s,buf,strlen(buf));
  }
  else
  {
    sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, e->x(), e->y());
    tcp_send(s,buf,strlen(buf));
  }
  //QToolBox::mousePressEvent(event);
}

void MyQToolBox::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  //QToolBox::mouseReleaseEvent(event);
}

void MyQToolBox::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QToolBox::enterEvent(event);
}

void MyQToolBox::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QToolBox::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyGroupBox::MyGroupBox(int *sock, int ident, int columns, Qt::Orientation o, QString title, QWidget * parent, const char * name)
           :QGroupBox(title,parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  if(columns == (int) o) return; // troll porting murx
}

MyGroupBox::~MyGroupBox()
{
}

void MyGroupBox::mousePressEvent(QMouseEvent *e)
{
  char buf[80];

  if(e->button() == Qt::RightButton)
  {
    sprintf(buf,"QMouseRight(%d)\n",id);
    tcp_send(s,buf,strlen(buf));
  }
  else
  {
    sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, e->x(), e->y());
    tcp_send(s,buf,strlen(buf));
  }
  //QGroupBox::mousePressEvent(event);
}

void MyGroupBox::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  //QGroupBox::mouseReleaseEvent(event);
}

void MyGroupBox::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QGroupBox::enterEvent(event);
}

void MyGroupBox::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  if(event == NULL) return;
  //QGroupBox::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyListBox::MyListBox(int *sock, int ident, QWidget *parent, const char *name)
          :QListWidget(parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(itemClicked(QListWidgetItem *)), SLOT(slotClicked(QListWidgetItem *)));
  connect(this, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), SLOT(slotCurrentChanged(QListWidgetItem *, QListWidgetItem *)));
  connect(this, SIGNAL(itemSelectionChanged()), SLOT(slotSendSelected()));
}

MyListBox::~MyListBox()
{
}

void MyListBox::insertItem(QPixmap &pixmap, const QString &text, int index)
{
  int row = index;
  QListWidgetItem *item;

  if(pixmap.isNull()) item = new QListWidgetItem(text);
  else                item = new QListWidgetItem(pixmap, text);
  if(row == -1) QListWidget::addItem(item);
  else          QListWidget::insertItem(row,item);
}

void MyListBox::changeItem(QPixmap &pixmap, const QString & text, int index)
{
  int row = index;
  QListWidgetItem *item;

  item = QListWidget::takeItem(row);
  if(item != NULL) delete item;
  if(pixmap.isNull()) item = new QListWidgetItem(text);
  else                item = new QListWidgetItem(pixmap, text);
  QListWidget::insertItem(row,item);
}

void MyListBox::removeItem(int index)
{
  int row = index;
  QListWidgetItem *item;

  item = QListWidget::item(row);
  if(item != NULL) delete item;
}

void MyListBox::removeItemByName(QString name)
{
  QList<QListWidgetItem *> list;

  QString txt = "*";
  list = findItems(txt,Qt::MatchWildcard);
  for(int i=0; i < list.size(); i++)
  {
    QListWidgetItem *item = list.at(i);
    if(item != NULL)
    {
      if(item->text() == name)
      {
        delete item;
        return;
      }
    }
  }
}

void MyListBox::clear()
{
  QListWidget::clear();
}

void MyListBox::slotClicked(QListWidgetItem *item)
{
char buf[80];

  if(item == NULL) return;
  sprintf(buf,"QListBox(%d,\"%s\")\n",id,decode(item->text()));
  tcp_send(s,buf,strlen(buf));
}

void MyListBox::slotCurrentChanged(QListWidgetItem *item, QListWidgetItem *previous)
{
char buf[80];

  if(item == NULL) return;
  sprintf(buf,"QListBox(%d,\"%s\")\n",id,decode(item->text()));
  tcp_send(s,buf,strlen(buf));
  if(previous == NULL) return;
}

void MyListBox::slotSendSelected()
{
char buf[MAX_PRINTF_LENGTH];
int i,cnt;

  cnt = count();
  for(i=0; i<cnt; i++)
  {
    QListWidgetItem *item = QListWidget::item(i);
    if(isItemSelected(item))
    {
      sprintf(buf,"selected(%d,%d,\"%s\")\n",id,i,decode(item->text()));
      tcp_send(s,buf,strlen(buf));
    }
  }
  sprintf(buf,"selected(%d,-1,\"(null)\")\n",id);
  tcp_send(s,buf,strlen(buf));
}

void MyListBox::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QListWidget::mousePressEvent(event);
}

void MyListBox::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QListWidget::mouseReleaseEvent(event);
}

void MyListBox::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QListWidget::enterEvent(event);
}

void MyListBox::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QListWidget::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyTable::MyTable(int *sock, int ident, int numRows, int numColumns, QWidget *parent, const char *name)
        :QTableWidget(numRows,numColumns,parent)
{
  s = sock;
  id = ident;
  wrap = 1;
  autoresize = 0;
#if QT_VERSION >= 0x040300                  
  setWordWrap(true);
#endif
  button = read_only = updates = 0;
  if(name != NULL) setObjectName(name);
  connect(horizontalHeader(), SIGNAL(sectionClicked(int)), SLOT(slotColClicked(int)));
  connect(verticalHeader()  , SIGNAL(sectionClicked(int)), SLOT(slotRowClicked(int)));
  connect(this, SIGNAL(cellClicked(int,int)), SLOT(slotClicked(int,int)));
  connect(this, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(slotCurrentChanged(int,int,int,int)));
  connect(this, SIGNAL(cellChanged(int,int)), SLOT(slotValueChanged(int,int)));
}

MyTable::~MyTable()
{
}

void MyTable::setEditable(int editable)
{
  for(int row=0; row<rowCount(); row++)
  {
    for(int column=0; column<columnCount(); column++)
    {
      QTableWidgetItem *tableitem = item(row,column);
      if(tableitem != NULL)
      {
        if(editable == 1) tableitem->setFlags(Qt::ItemIsEditable);
        else              tableitem->setFlags(Qt::ItemIsEnabled);
      }
    }
  }
}

void MyTable::setTableButton(int row, int col, QString text)
{
  int r,g,b;
  r = g = b = -2;
  if(text.startsWith("color("))
  {
    sscanf(text.toAscii(),"color(%d,%d,%d",&r,&g,&b);
    text = text.section(')',1);
  }
  MyQPushButton *button = new MyQPushButton(s,id,0);
  button->setText(text);
  button->row = row;
  button->col = col;
  setCellWidget(row,col,button);
  if(r!=-2 && g!=-2 && b!=-2) mySetBackgroundColor(button,TQPushButton,r,g,b);
}

void MyTable::setTableCheckBox(int row, int col, int state, QString text)
{
  int r,g,b;
  r = g = b = -2;
  if(text.startsWith("color("))
  {
    sscanf(text.toAscii(),"color(%d,%d,%d",&r,&g,&b);
    text = text.section(')',1);
  }
  MyCheckBox *check = new MyCheckBox(s,id,0);
  check->setText(text);
  if(state) check->setCheckState(Qt::Checked);
  else      check->setCheckState(Qt::Unchecked);
  check->row = row;
  check->col = col;
  setCellWidget(row,col,check);
  if(r!=-2 && g!=-2 && b!=-2) mySetBackgroundColor(check,TQCheck,r,g,b);
}

void MyTable::setTableComboBox(int row, int col, int editable, const char *menu)
{
  char buf[800];
  int  i,ifirst;
  QStringList list;
  ifirst = 0;
  for(i=0;; i++)
  {
    if(menu[i] == ',' || menu[i] == '\0')
    {
      if(i == ifirst && i == 0)
      {
      }
      else if(menu[i] != '\0' && menu[i+1] == ',')
      {
        if(i > 0 && menu[i-1] != ',')
        {
          strncpy(buf,&menu[ifirst],i-ifirst);
          buf[i-ifirst] = '\0';
          list.append(buf);
        }
      }
      else
      {
        if(i > 0 && menu[i-1] != ',')
        {
          strncpy(buf,&menu[ifirst],i-ifirst);
          buf[i-ifirst] = '\0';
          list.append(buf);
        }
      }
      ifirst = i+1;
    }
    if(menu[i] == '\0') break;
  }

  MyComboBox *combo = new MyComboBox(s,id,0);
  combo->addItems(list);
  combo->setEditable(editable);
  combo->row = row;
  combo->col = col;
  setCellWidget(row,col,combo); 
  //printf("ComboBox(%d,%d,%d,%s)\n",row,col,editable,text);
}

void MyTable::setTableLabel(int row, int col, QString text)
{
  MyLabel *label = new MyLabel(s,id,0);
  label->setText(text);
  label->row = row;
  label->col = col;
  setCellWidget(row,col,label);
}

void MyTable::slotRowClicked(int section)
{
char buf[80];

  sprintf(buf,"QTable(%d,%d,%d,%d)\n",id,section,-1,1);
  tcp_send(s,buf,strlen(buf));
}

void MyTable::slotColClicked(int section)
{
char buf[80];

  sprintf(buf,"QTable(%d,%d,%d,%d)\n",id,-1,section,1);
  tcp_send(s,buf,strlen(buf));
}

void MyTable::mousePressEvent(QMouseEvent *event)
{
  //char buf[80];

  updates = 0;
  if     (event->button() == Qt::LeftButton)  button = 1;
  else if(event->button() == Qt::MidButton)   button = 2;
  else if(event->button() == Qt::RightButton) button = 3;
  else                                        button = 0;
  //if(button == 3)
  //{
  //  sprintf(buf,"QMouseRight(%d)\n",id);
  //  tcp_send(s,buf,strlen(buf));
  //}

  if(button == 3)
  {
    QMenu popupMenu;
    QAction *ret;
    QString buf;

    popupMenu.addAction("Save table as CSV file");
    if(opt.view_csv[0] != '\0')
    {
      buf.sprintf("Open table with %s", opt.view_csv);
      popupMenu.addAction(buf);
    }  
    ret = popupMenu.exec(QCursor::pos());
    if(ret != NULL)
    {
      if(ret->text().startsWith("Save")) 
      {
        saveTextfile();
      }  
      else
      {
        char buf[1024];
        saveTextfile("table.csv");
        if(strlen(opt.view_csv) < 900)
        {
          strcpy(buf, opt.view_csv);
          strcat(buf, " table.csv");
#ifndef PVWIN32
          strcat(buf, " &");
#endif
          if(strlen(opt.view_csv) >= 3) mysystem(buf);
        }  
      }  
    }
  }  
  QTableWidget::mousePressEvent(event);
}

//void MyTable::slotClicked(int row, int col, int button)
void MyTable::slotClicked(int row, int col)
{
char buf[80];

  sprintf(buf,"QTable(%d,%d,%d,%d)\n",id,row,col,button);
  tcp_send(s,buf,strlen(buf));
}

void MyTable::slotCurrentChanged( int row, int col, int oldrow, int oldcol)
{
char buf[80];

  sprintf(buf,"QTable(%d,%d,%d,0)\n",id,row,col);
  tcp_send(s,buf,strlen(buf));
  if(oldrow == oldcol) return;
}

void MyTable::slotValueChanged(int row, int col)
{
  char buf[MAX_PRINTF_LENGTH];
  const char *cptr;

  if(opt.echo_table_updates == 0)
  {
    if(updates > 0)
    {
      if(opt.arg_debug) printf("MyTable::slotValueChanged: updates=%d\n", updates);
      updates--;
      return;
    }
  }  
  updates = 0;
  if(opt.arg_debug) printf("MyTable::slotValueChanged\n");
  QString txt;
  QString celltext = item(row,col)->text();
  if(celltext.startsWith("color("))
  {
    txt = celltext.mid(1+celltext.indexOf(')'));
  }
  else
  {
    txt = item(row,col)->text();
  }
  cptr = decode(txt);
  if(strlen(cptr) > MAX_PRINTF_LENGTH-40) return;
  sprintf(buf,"QTableValue(%d,%d,%d,\"%s\")\n",id,row,col,cptr);
  tcp_send(s,buf,strlen(buf));
}

void MyTable::saveTextfile(const char *filename)
{
  QFileDialog dlg;
  QString name,cell;
  FILE *fp;
  int x,y,ret;

  if(filename == NULL)
  {
    name = dlg.getSaveFileName(NULL,QString::null,QString::null,"*.csv");
  }
  else
  {
    name = filename;
  }
  if(name.isEmpty()) return;
  if(filename == NULL)
  {
    fp = fopen(name.toAscii(),"r");
    if(fp != NULL)
    {
      fclose(fp);
      ret = QMessageBox::warning(this,"Save Table","File already exists: Overwrite ?",QMessageBox::Yes,QMessageBox::No,0);
      if(ret == QMessageBox::No) return;
    }
  }  
  fp = fopen(name.toAscii(),"w");
  if(fp == NULL)
  {
    QMessageBox::warning(this,"Save Table","could not write file",QMessageBox::Ok,0,0);
    return;
  }
  for(y=-1; y<rowCount(); y++)
  {
    if(y == -1)                              cell = "";
    else if(horizontalHeaderItem(y) == NULL) cell = "";
    else                                     cell = horizontalHeaderItem(y)->text();
    if(cell.isEmpty())                       cell = "";
    else                                     fprintf(fp,"%s",(const char *) cell.toUtf8());
    for(x=0; x<columnCount(); x++)
    {
      if(y == -1)
      {
        if(verticalHeaderItem(x) == NULL) cell = "";
        else                              cell = verticalHeaderItem(x)->text();
      }
      else if(item(y,x) == NULL) cell = "";
      else                       cell = item(y,x)->text();
      if(cell.isEmpty())         cell = "";
      fprintf(fp,"\t%s",(const char *) cell.toUtf8());
    }
    fprintf(fp,"\n");
  }
  fclose(fp);
}

void MyTable::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QTableWidget::enterEvent(event);
}

void MyTable::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QTableWidget::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MySpinBox::MySpinBox(int *sock, int ident, int minValue, int maxValue, int step, QWidget *parent, const char *name)
          :QSpinBox(parent)
{
  s = sock;
  id = ident;
  setMinimum(minValue);
  setMaximum(maxValue);
  setSingleStep(step);
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)));
}

MySpinBox::~MySpinBox()
{
}

void MySpinBox::slotValueChanged(int value)
{
char buf[80];

  sprintf(buf,"slider(%d,%d)\n",id,value);
  tcp_send(s,buf,strlen(buf));
}

void MySpinBox::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QSpinBox::mousePressEvent(event);
}

void MySpinBox::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QSpinBox::mouseReleaseEvent(event);
}

void MySpinBox::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QSpinBox::enterEvent(event);
}

void MySpinBox::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QSpinBox::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyDial::MyDial(int *sock, int ident, int minValue, int maxValue, int pageStep, int value, QWidget *parent, const char *name)
       :QDial(parent)
{
  s = sock;
  id = ident;
  setMaximum(maxValue);
  setMinimum(minValue);
  setPageStep(pageStep);
  setValue(value);
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)));
}

MyDial::~MyDial()
{
}

void MyDial::setValue(int value)
{
  QDial::setValue(value);
}

void MyDial::slotValueChanged(int value)
{
char buf[80];

  sprintf(buf,"slider(%d,%d)\n",id,value);
  tcp_send(s,buf,strlen(buf));
}

void MyDial::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QDial::mousePressEvent(event);
}

void MyDial::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QDial::mouseReleaseEvent(event);
}

void MyDial::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QDial::enterEvent(event);
}

void MyDial::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QDial::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyProgressBar::MyProgressBar(int *sock, int ident, int totalSteps, QWidget *parent, const char *name)
              :QProgressBar(parent)
{
  s = sock;
  id = ident;
  setMaximum(totalSteps);
  setObjectName(name);
}

MyProgressBar::~MyProgressBar()
{
}

void MyProgressBar::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QProgressBar::mousePressEvent(event);
}

void MyProgressBar::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QProgressBar::mouseReleaseEvent(event);
}

void MyProgressBar::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QProgressBar::enterEvent(event);
}

void MyProgressBar::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QProgressBar::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyMultiLineEdit::MyMultiLineEdit(int *sock, int ident, int editable, int maxLines, QWidget *parent, const char *name)
                :QTextEdit(parent)
{
  s = sock;
  id = ident;
  maxlines = maxLines;
  num_lines = 0;
  if(name != NULL) setObjectName(name);
  if     (editable == 0) setReadOnly(true);
  else if(editable == 1) setReadOnly(false);
  setLineWrapMode(QTextEdit::NoWrap);
}

MyMultiLineEdit::~MyMultiLineEdit()
{
}

void MyMultiLineEdit::setText(const QString &text)
{
  /* trollmurx
  if(maxlines != -1)
  {
    if(lines() >= maxlines)
    {
      removeParagraph(0);
    }
  }
  setCursorPosition(lines()-1,paragraphLength(lines()-1));
  */
  if(maxlines != -1)
  {
    if(num_lines >= maxlines)
    {
      QFont  font  = currentFont();
      QColor color = textColor();
      QTextCursor cursor = textCursor();
      cursor.movePosition(QTextCursor::Start);
      cursor.clearSelection();
      cursor.select(QTextCursor::BlockUnderCursor);
      cursor.removeSelectedText();
      cursor.deleteChar();
      cursor.movePosition(QTextCursor::End);
      setCurrentFont(font);
      setTextColor(color);
      num_lines--;
    }
  }
  append(text);
  num_lines++;
  textCursor().movePosition(QTextCursor::End);
}

void MyMultiLineEdit::slotSendToClipboard()
{
  char buf[80];
  const char *cptr;
  int len;

  cptr = document()->toPlainText().toUtf8().constData();
  len = 0;
  while(cptr[len] != '\0') len++;
  sprintf(buf,"@clipboard(%d,%d)\n", id,len);
  tcp_send(s,buf,strlen(buf));
  tcp_send(s,cptr,len);
}

void MyMultiLineEdit::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QTextEdit::mousePressEvent(event);
}

void MyMultiLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QTextEdit::mouseReleaseEvent(event);
}

void MyMultiLineEdit::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QTextEdit::enterEvent(event);
}

void MyMultiLineEdit::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QTextEdit::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyTextBrowser::MyTextBrowser(int *sock, int ident, QWidget *parent, const char *name)
              :QWebView(parent)
{
  s = sock;
  id = ident;
  homeIsSet = 0;
  if(name != NULL) setObjectName(name);
  page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  connect(this, SIGNAL(linkClicked(const QUrl &)), SLOT(slotLinkClicked(const QUrl &)));
  //connect(this, SIGNAL(urlChanged(const QUrl &)), SLOT(slotUrlChanged(const QUrl &)));
}

MyTextBrowser::~MyTextBrowser()
{
}

void MyTextBrowser::moveContent(int pos)
{
  char buf[MAX_PRINTF_LENGTH];
  QString myurl;
  QWebHistory *hist;

  if(opt.arg_debug) printf("moveContent(%d)\n", pos);
  if     (pos == 0 && homeIsSet) 
  { 
    myurl = home; 
    load(home); 
  }
  else if(pos == 1)              
  { 
    hist = history();
    if(hist != NULL && hist->canGoForward()) myurl = hist->forwardItem().url().toString(); 
    forward();
  }  
  else if(pos == 2) 
  {
    hist = history();
    if(hist != NULL && hist->canGoBack()) myurl = hist->backItem().url().toString(); 
    back();
  }  
  else if(pos == 3) 
  {
    hist = history();
    if(hist != NULL) myurl = hist->currentItem().url().toString(); 
    reload();
  }  

  if(myurl.isEmpty()) return;
  if(opt.arg_debug) printf("moveContent(%s)\n", (const char *) myurl.toAscii());
  if(myurl.length()+40 > MAX_PRINTF_LENGTH) return;
  sprintf(buf,"text(%d,\"%s\")\n", id,decode(myurl));
  tcp_send(s,buf,strlen(buf));
}

void MyTextBrowser::setHTML(QString &text)
{
  int i;

  // replace href="/any_string" (href to file starting from root directory) by href="awite://any_string"
  while(1)
  {
    i = text.indexOf("href=\"/");
    if(i < 0) break;
    text = text.replace(i,7,"href=\"awite://");
  }

  if(opt.arg_debug) printf("MyTextBrowser::setHTML:: %s\n", (const char *) text.toAscii());
  setHtml(text);
}

void MyTextBrowser::slotLinkClicked(const QUrl &link)
{
  char buf[MAX_PRINTF_LENGTH];
  QString url;
  int i;

  url = link.toString();

  // replace "href=\"//awite://" by "href=/"
  while(1)
  {
    i = url.indexOf("awite://");
    if(i < 0) break;
    url = url.replace(i,8,"/");
    if(opt.arg_debug) printf("MyTextBrowser::slotLinkClicked::link clicked = %s\n", (const char *) url.toAscii());
  }

  if(opt.arg_debug) printf("slotLinkClicked(%s)\n", (const char *) url.toAscii());
  if(url.length()+40 > MAX_PRINTF_LENGTH) return;
  sprintf(buf,"text(%d,\"%s\")\n", id,decode(url));
  tcp_send(s,buf,strlen(buf));
  load(link);
}

void MyTextBrowser::slotUrlChanged(const QUrl &link)
{
  char buf[MAX_PRINTF_LENGTH];
  QString url;

  url = link.toString();
  if(opt.arg_debug) printf("slotUrlChanged(%s)\n", (const char *) url.toAscii());
  if(url.length()+40 > MAX_PRINTF_LENGTH) return;
  sprintf(buf,"text(%d,\"%s\")\n", id,decode(url));
  tcp_send(s,buf,strlen(buf));
}

void MyTextBrowser::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QWebView::mousePressEvent(event);
}

void MyTextBrowser::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QWebView::mouseReleaseEvent(event);
}

void MyTextBrowser::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QWebView::enterEvent(event);
}

void MyTextBrowser::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QWebView::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
static int starts_with(const char *path1, const char *path2)
{
  int i;
  for(i=0; path1[i] != '\0'; i++)
  {
    if(path2[i] == '\0')
    {
      if(path1[i] == '/') return 1;
      else                return 0;
    }
    if(path1[i] != path2[i]) return 0;
  }
  if(path2[i] != '/' && path2[i] != '\0') return 0; // RL 15.10.2004
  return 1;
}

const char *root_path(const char *path, int num_slash)
{
  static char buf[1024];
  int i,cnt;

  i   = 0;
  cnt = -1;
  while(i < (int) sizeof(buf))
  {
    if(path[i] == '/') cnt++;
    if(cnt == num_slash) break;
    if(path[i] == '\0')  break;
    buf[i] = path[i];
    i++;
  }
  buf[i] = '\0';
  return buf;
}

MyListView::MyListView(int *sock, int ident, QWidget *parent, const char *name)
           :QTreeWidget(parent)
{
  s = sock;
  id = ident;
  recursion = icol = 0;
  if(name != NULL) setObjectName(name);
  setSortingEnabled(false);
  connect(this, SIGNAL(itemClicked(QTreeWidgetItem *, int)), SLOT(slotClicked(QTreeWidgetItem *, int)));
  connect(this, SIGNAL(itemSelectionChanged()), SLOT(slotSendSelected()));
  headerItem()->setHidden(false);
  for(int i=0; i<20; i++) colwidth[i] = 100;
}

MyListView::~MyListView()
{
}

void MyListView::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::RightButton)
  {
    if(opt.arg_debug) printf("rightButtonPressed\n");
    //QPoint parent0 = mapToGlobal(QPoint(0,0));
    //int x = event->x() - parent0.x();
    //int y = event->y() - parent0.y();
    //MyListViewItem *item = (MyListViewItem *) childAt(x,y);
    slotRightButtonPressed(NULL, currentColumn());
  }
  else
  {
    char buf[80];
    sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
    tcp_send(s,buf,strlen(buf));
  }
  QTreeWidget::mousePressEvent(event);
}

void MyListView::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QTreeWidget::mouseReleaseEvent(event);
}

void MyListView::addColumn(QString text, int size)
{
  if(opt.arg_debug) printf("addColumn(%s) icol=%d\n",(const char *) text.toAscii(), icol);
  setColumnCount(icol+1);
  headerItem()->setText(icol,text);
  if(size > 0)
  {
    if(opt.arg_debug) printf("addColumn: icol=%d size=%d currentColumn=%d columnCount=%d\n", icol, size, currentColumn(), columnCount());
    //QSize qsize;
    //qsize.setWidth(size);
    //qsize.setHeight(20);
    //headerItem()->setSizeHint(icol,qsize);
    if(icol >= 0 && icol < 20)
    {
      colwidth[icol] = size;
      for(int i=0; i<columnCount(); i++)
      {
        setColumnWidth(i, colwidth[i]);
      }
    }  
  }
  setColumnHidden(icol,false);
  icol++;
}

void MyListView::setMultiSelection(int mode)
{
  // mode := 0=SingleSelection|1=MultiSelection|2=NoSelection
  if(mode == 0)
  {
    setSelectionMode(QAbstractItemView::SingleSelection);
  }
  else if(mode == 1)
  {
    setSelectionMode(QAbstractItemView::MultiSelection);
  }
  else
  {
    setSelectionMode(QAbstractItemView::NoSelection);
  }
}

void MyListView::setSorting(int col, int mode)
{
  //Sort column
  //mode=0 decending
  //mode=1 ascendin
  //column=-1 do not allow sorting (this is the default)
  //Allowed widgets QListView
  if(col < 0)
  {
    setSortingEnabled(false);
  }
  else
  {
    setSortingEnabled(true);
    if(mode == 0)
    {
      sortItems(col,Qt::DescendingOrder);
    }
    else
    {
      sortItems(col,Qt::AscendingOrder);
    }
  }
}

MyListViewItem *MyListView::firstChild(MyListViewItem *parent)
{
  MyListViewItem *item;
  if(recursion >= MAX_TREE_RECURSION) return NULL;
  ichild[recursion] = 0;
  if(parent == NULL) // child of ListView
  {
    item = (MyListViewItem *) topLevelItem(ichild[recursion]++);
  }
  else               // child of ListViewItem
  {
    item = NULL;
    if(parent->childCount() > 0) item = (MyListViewItem *) parent->child(ichild[recursion]++);
  }
  return item;
}

MyListViewItem *MyListView::nextSibling(MyListViewItem *sibling, QTreeWidgetItem *parent)
{
  MyListViewItem *item = NULL;
  if(sibling == NULL) return NULL;
  if(parent == NULL) // child of ListView
  {
    if(topLevelItemCount() >= ichild[recursion]) item = (MyListViewItem *) topLevelItem(ichild[recursion]++);
  }
  else                                   // child of ListViewItem
  {
    if(parent->childCount() >= ichild[recursion]) item = (MyListViewItem *) parent->child(ichild[recursion]++);
  }
  return item;
}

void MyListView::insertItem(MyListViewItem *item, MyListViewItem *parent, int num_slash)
{
  if(num_slash == 1) // child of ListView
  {
    addTopLevelItem(item);
  }
  else               // child of ListViewItem
  {
    parent->addChild(item);
  }
}

void MyListView::nameVersionSetListViewText(const char *path, int column, QString &text, MyListViewItem *parent, const char *relpath, int num_slash)
{
  MyListViewItem *item;
  const char *cptr;

  item = firstChild(parent);
  while(item != NULL)
  {
    if(strcmp(path,item->path.toAscii()) == 0) { item->setText(column,text); return; } // update existing item
    if(starts_with(path,item->path.toAscii()))
    {
      cptr = strchr(&relpath[1],'/');
      if(cptr == NULL) return;
      recursion++;
      nameVersionSetListViewText(path,column,text,item,cptr,num_slash+1); // recurse
      recursion--;
      return;
    }
    item = nextSibling(item,parent);
  }
  if(num_slash == 1)   item = new MyListViewItem((MyListView *) NULL); // add root path
  else                 item = new MyListViewItem((MyListViewItem *) NULL); // add root path
  item->path = root_path(path,num_slash);
  insertItem(item,parent,num_slash);
  if(item->path == path) { item->setText(column,text); return; }
  cptr = strchr(&relpath[1],'/');
  if(cptr == NULL) return;
  recursion++;
  nameVersionSetListViewText(path,column,text,item,cptr,num_slash+1); // recurse
  recursion--;
  return;
}

void MyListView::nameVersionSetListViewPixmap(const char *path, int column, QPixmap &pixmap, MyListViewItem *parent, const char *relpath, int num_slash)
{
  MyListViewItem *item;
  const char *cptr;

  item = firstChild(parent);
  while(item != NULL)
  {
    if(strcmp(path,item->path.toAscii()) == 0) { item->setIcon(column,pixmap); return; }  // update existing item
    if(starts_with(path,item->path.toAscii()))
    {
      cptr = strchr(&relpath[1],'/');
      if(cptr == NULL) return;
      recursion++;
      nameVersionSetListViewPixmap(path,column,pixmap,item,cptr,num_slash+1);     // recurse
      recursion--;
      return;
    }
    item = nextSibling(item,parent);
  }
  if(num_slash == 1)   item = new MyListViewItem((MyListView *)NULL);     // add root path
  else                 item = new MyListViewItem((MyListViewItem *)NULL);   // add root path
  item->path = root_path(path,num_slash);
  insertItem(item,parent,num_slash);
  if(item->path == path) { item->setIcon(column,pixmap); return; }
  cptr = strchr(&relpath[1],'/');
  if(cptr == NULL) return;
  recursion++;
  nameVersionSetListViewPixmap(path,column,pixmap,item,cptr,num_slash+1);         // recurse
  recursion--;
  return;
}

void MyListView::setListViewText(const char *path, int column, QString &text)
{
  if(path[0] != '/') return;
  //triggerUpdate(); //rlehrig not necessary ?
  recursion = 0;
  nameVersionSetListViewText(path,column,text,NULL,path,1);
}

void MyListView::setListViewPixmap(const char *path, int column, QPixmap &pixmap)
{
  if(path[0] != '/') return;
  //triggerUpdate(); //rlehrig not necessary ?
  recursion = 0;
  nameVersionSetListViewPixmap(path,column,pixmap,NULL,path,1);
}

int MyListView::deleteListViewItem(const char *path, MyListViewItem *item)
{
  MyListViewItem *child;
  int ret;

  while(item != NULL)
  {
    if(strcmp(item->path.toAscii(),path) == 0)
    {
      delete item;
      return 1;
    }
    recursion++;
    child = firstChild(item);
    if(child != NULL)
    {
      ret = deleteListViewItem(path,child);
      if(ret != 0)
      {
        recursion--;
        return ret;
      }
    }
    recursion--;
    item = nextSibling(item,item->parent());
  }
  return 0;
}

int MyListView::ensureVisible(const char *path, MyListViewItem *item)
{
  MyListViewItem *child;
  int ret;

  while(item != NULL)
  {
    if(strcmp(item->path.toAscii(),path) == 0)
    {
      scrollToItem(item, QAbstractItemView::EnsureVisible);
      return 1;
    }
    recursion++;
    child = firstChild(item);
    if(child != NULL)
    {
      ret = ensureVisible(path,child);
      if(ret != 0)
      {
        recursion--;
        return ret;
      }
    }
    recursion--;
    item = nextSibling(item,item->parent());
  }
  return 0;
}

int MyListView::setItemOpen(const char *path, int open, MyListViewItem *item)
{
  MyListViewItem *child;
  int ret;

  while(item != NULL)
  {
    if(strcmp(item->path.toAscii(),path) == 0)
    {
      setItemExpanded(item, (bool) open);
      return 1;
    }
    recursion++;
    child = firstChild(item);
    if(child != NULL)
    {
      ret = setItemOpen(path,open,child);
      if(ret != 0)
      {
        recursion--;
        return ret;
      }
    }
    recursion--;
    item = nextSibling(item,item->parent());
  }
  return 0;
}

void MyListView::closeTree(MyListViewItem *lvi, int mode)
{
  recursion++;
  if(lvi)
  {
    if(mode==2)  setItemExpanded(lvi, false);
    setItemSelected(lvi,false);
    //rlehrig not necessary ? lvi->repaint();
    closeTree(firstChild(lvi), mode);
    closeTree(nextSibling(lvi,lvi->parent()), mode);
  }
  recursion--;
  return;
}

void MyListView::setSelected(int mode, const char *path)
{
  const char *ptr;

  ptr = &path[1];
  MyListViewItem *plvi;
  MyListViewItem *lvi = firstChild(NULL);

  plvi=lvi;
  closeTree(plvi, mode);

  while((ptr=strchr(ptr, '/')))
  {
    do
    {
      if((!strncmp((const char *) plvi->path.toAscii(), path, ptr-(const char *)path)) &&
          (strlen((const char *) plvi->path.toAscii())==(unsigned)(ptr-(const char *)path)))
      {
        setItemExpanded(plvi, (bool) mode);
        break;
      }
      plvi = nextSibling(plvi,plvi->parent());
    }
    while(plvi);
    if(!plvi) break;
    plvi = firstChild(plvi);
    ptr++;
  }

  while(plvi)
  {
    if(!strcmp((const char *) plvi->path.toAscii(), path))
    {
      setItemExpanded(plvi, (bool) mode); //plvi->setOpen(mode);
      setItemSelected(plvi, (bool) mode);
      doSendSelected(plvi);
      break;
    }
    plvi = nextSibling(plvi,plvi->parent());
  }

  //rllehrig not necessary ? repaint();
}

void MyListView::slotClicked(QTreeWidgetItem *item, int column)
{
  char buf[MAX_PRINTF_LENGTH];
  MyListViewItem *myitem = (MyListViewItem *) item;

  if(item == NULL) return;
  if(opt.arg_debug) printf("clicked\n");
  if(opt.arg_debug) printf("path=%s\n",(const char *) myitem->path.toAscii());
  int col = icol; //columnCount();
  while(col > 0)
  {
    col--;
    //sprintf(buf,"text(%d,\"%s\")\n", id, (const char *) item->text(col));
    sprintf(buf,"selected(%d,%d,\"%s\")\n", id, col,decode(myitem->text(col)));
    tcp_send(s,buf,strlen(buf));
  }
  sprintf(buf,"selected(%d,-1,\"%s\")\n", id, decode(myitem->path));
  tcp_send(s,buf,strlen(buf));
  if(column == -1000) return;
}

void MyListView::slotSendSelected()
{
char buf[MAX_PRINTF_LENGTH];

  recursion = 0;
  doSendSelected(firstChild(NULL));
  sprintf(buf,"selected(%d,-2,\"(null)\")\n",id);
  tcp_send(s,buf,strlen(buf));
}

void MyListView::doSendSelected(MyListViewItem *item)
{
char buf[MAX_PRINTF_LENGTH];
int column;

  while(item != NULL)
  {
    if(isItemSelected(item))
    {
      column = icol; //columnCount();
      while(column > 0)
      {
        column--;
        if(opt.arg_debug) printf("doSendSelected column=%d text=%s\n",column,decode(item->text(column)));
        sprintf(buf,"selected(%d,%d,\"%s\")\n", id, column, decode(item->text(column)));
        tcp_send(s,buf,strlen(buf));
      }
      MyListViewItem *myitem = (MyListViewItem *) item;
      sprintf(buf,"selected(%d,-1,\"%s\")\n", id, decode(myitem->path));
      if(opt.arg_debug) printf("doSendSelected path=%s\n",decode(myitem->path));
      tcp_send(s,buf,strlen(buf));
    }
    recursion++;
    if(firstChild(item) != NULL)
    {
      doSendSelected(firstChild(item));
    }
    recursion--;
    item = nextSibling(item,item->parent());
  }
}

void MyListView::slotRightButtonPressed(QTreeWidgetItem *item, int column)
{
  char buf[80];

  MyListViewItem *myitem = (MyListViewItem *) item;
  if(item == NULL)
  {
    sprintf(buf,"QMouseRight(%d,\"%d,\")\n",id,column);
  }
  else
  {
    sprintf(buf,"QMouseRight(%d,\"%d,%s\")\n",id,column, decode(myitem->path));
  }
  tcp_send(s,buf,strlen(buf));
}

void MyListView::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QTreeWidget::enterEvent(event);
}

void MyListView::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QTreeWidget::leaveEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
MyIconView::MyIconView(int *sock, int ident, QWidget *parent, const char *name)
           :QListWidget(parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  QListView::setViewMode(QListView::IconMode);
  setDragDropMode(QAbstractItemView::NoDragDrop);
  connect(this, SIGNAL(itemClicked(QListWidgetItem *)), SLOT(slotClicked(QListWidgetItem *)));
}

MyIconView::~MyIconView()
{
}

void MyIconView::setIconViewItem(QString &text, QPixmap &pixmap)
{
  QListWidgetItem *item;

  if(pixmap.isNull()) item = new QListWidgetItem(text);
  else                item = new QListWidgetItem(pixmap, text);
  QListWidget::addItem(item);
}

void MyIconView::deleteIconViewItem(QString &text)
{
  QList<QListWidgetItem *> list;

  if(text.isEmpty()) return;
  QString txt = "*";
  list = findItems(txt,Qt::MatchWildcard);
  for(int i=0; i < list.size(); i++)
  {
    QListWidgetItem *item = list.at(i);
    if(item != NULL)
    {
      if(item->text() == text)
      {
        delete item;
        return;
      }
    }
  }
}

void MyIconView::slotClicked(QListWidgetItem *item)
{
char buf[MAX_PRINTF_LENGTH];

  if(item == NULL) return;
  if(item->text().length()+40 > MAX_PRINTF_LENGTH) return;
  sprintf(buf,"text(%d,\"%s\")\n", id, decode(item->text()));
  tcp_send(s,buf,strlen(buf));
}

void MyIconView::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QListWidget::mousePressEvent(event);
}

void MyIconView::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QListWidget::mouseReleaseEvent(event);
}

void MyIconView::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QListWidget::enterEvent(event);
}

void MyIconView::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QListWidget::leaveEvent(event);
}

#ifndef NO_QWT
// --- QWT --------------------------------------------------------------
MyQwtScale::MyQwtScale(int *sock, int ident, int position, QWidget *parent, const char *name)
           :QwtScaleWidget((QwtScaleDraw::Alignment)position,parent)
{
  s = sock;
  id = ident;
  if(position == 0) setAlignment((QwtScaleDraw::Alignment) Qt::Horizontal);
  else              setAlignment((QwtScaleDraw::Alignment) Qt::Vertical);
  if(name != NULL)  setObjectName(name);
}

MyQwtScale::~MyQwtScale()
{
}

void MyQwtScale::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QwtScaleWidget::mousePressEvent(event);
}

void MyQwtScale::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QwtScaleWidget::mouseReleaseEvent(event);
}

void MyQwtScale::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtScaleWidget::enterEvent(event);
}

void MyQwtScale::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtScaleWidget::leaveEvent(event);
}

MyQwtThermo::MyQwtThermo(int *sock, int ident, QWidget *parent, const char *name)
            :QwtThermo(parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
}

MyQwtThermo::~MyQwtThermo()
{
}

void MyQwtThermo::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QwtThermo::mousePressEvent(event);
}

void MyQwtThermo::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QwtThermo::mouseReleaseEvent(event);
}

void MyQwtThermo::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtThermo::enterEvent(event);
}

void MyQwtThermo::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtThermo::leaveEvent(event);
}

MyQwtKnob::MyQwtKnob(int *sock, int ident, QWidget *parent, const char *name)
          :QwtKnob(parent)
{
  s = sock;
  id = ident;
  connect(this, SIGNAL(valueChanged(double)), SLOT(slotValueChanged(double)));
  if(name != NULL) setObjectName(name);
}

MyQwtKnob::~MyQwtKnob()
{
}

void MyQwtKnob::slotValueChanged(double value)
{
  char buf[80];

  if(opt.arg_debug) printf("KnobValue=%f\n", (float) value);
  sprintf(buf,"slider(%d,%lf)\n",id,value);
  tcp_send(s,buf,strlen(buf));
}

void MyQwtKnob::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QwtKnob::mousePressEvent(event);
}

void MyQwtKnob::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QwtKnob::mouseReleaseEvent(event);
}

void MyQwtKnob::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtKnob::enterEvent(event);
}

void MyQwtKnob::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtKnob::leaveEvent(event);
}

MyQwtCounter::MyQwtCounter(int *sock, int ident, QWidget *parent, const char *name)
             :QwtCounter(parent)
{
  s = sock;
  id = ident;
  connect(this, SIGNAL(valueChanged(double)), SLOT(slotValueChanged(double)));
  if(name != NULL) setObjectName(name);
}

MyQwtCounter::~MyQwtCounter()
{
}

void MyQwtCounter::slotValueChanged(double value)
{
char buf[80];

  sprintf(buf,"slider(%d,%lf)\n",id,value);
  tcp_send(s,buf,strlen(buf));
}

void MyQwtCounter::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QwtCounter::mousePressEvent(event);
}

void MyQwtCounter::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QwtCounter::mouseReleaseEvent(event);
}

void MyQwtCounter::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtCounter::enterEvent(event);
}

void MyQwtCounter::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtCounter::leaveEvent(event);
}

MyQwtWheel::MyQwtWheel(int *sock, int ident, QWidget *parent, const char *name)
           :QwtWheel(parent)
{
  s = sock;
  id = ident;
  setRange(0, 100);
  setValue(0.0);
  setMass(0.2);
  setTotalAngle(360.0);
  connect(this, SIGNAL(valueChanged(double)), SLOT(slotValueChanged(double)));
  if(name != NULL) setObjectName(name);
}

MyQwtWheel::~MyQwtWheel()
{
}

void MyQwtWheel::slotValueChanged(double value)
{
char buf[80];

  sprintf(buf,"slider(%d,%lf)\n",id,value);
  tcp_send(s,buf,strlen(buf));
}

void MyQwtWheel::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QwtWheel::mousePressEvent(event);
}

void MyQwtWheel::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QwtWheel::mouseReleaseEvent(event);
}

void MyQwtWheel::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtWheel::enterEvent(event);
}

void MyQwtWheel::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtWheel::leaveEvent(event);
}

MyQwtSlider::MyQwtSlider(int *sock, int ident, QWidget *parent, const char *name)
            :QwtSlider(parent)
{
  s = sock;
  id = ident;
  connect(this, SIGNAL(valueChanged(double)), SLOT(slotValueChanged(double)));
  if(name != NULL) setObjectName(name);
}

MyQwtSlider::~MyQwtSlider()
{
}

void MyQwtSlider::slotValueChanged(double value)
{
  char buf[80];

  sprintf(buf,"slider(%d,%lf)\n",id,value);
  tcp_send(s,buf,strlen(buf));
}

void MyQwtSlider::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QwtSlider::mousePressEvent(event);
}

void MyQwtSlider::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QwtSlider::mouseReleaseEvent(event);
}

void MyQwtSlider::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtSlider::enterEvent(event);
}

void MyQwtSlider::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtSlider::leaveEvent(event);
}

MyQwtCompass::MyQwtCompass(int *sock, int ident, QWidget *parent, const char *name)
             :QwtCompass(parent)
{
  s = sock;
  id = ident;
  connect(this, SIGNAL(valueChanged(double)), SLOT(slotValueChanged(double)));
  if(name != NULL) setObjectName(name);
}

MyQwtCompass::~MyQwtCompass()
{
}

void MyQwtCompass::slotValueChanged(double value)
{
  char buf[80];

  sprintf(buf,"slider(%d,%lf)\n",id,value);
  tcp_send(s,buf,strlen(buf));
}

void MyQwtCompass::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QwtCompass::mousePressEvent(event);
}

void MyQwtCompass::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QwtCompass::mouseReleaseEvent(event);
}

void MyQwtCompass::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtCompass::enterEvent(event);
}

void MyQwtCompass::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtCompass::leaveEvent(event);
}

#endif // #ifndef NO_QWT

MyQDateEdit::MyQDateEdit(int *sock, int ident, QWidget *parent, const char *name)
            :QDateEdit(parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(dateChanged(const QDate &)), SLOT(slotValueChanged(const QDate &)));
}

MyQDateEdit::~MyQDateEdit()
{
}

void MyQDateEdit::slotValueChanged(const QDate &date)
{
char buf[80];

  sprintf(buf,"text(%d,\"%d:%d:%d\")\n", id, date.year(), date.month(), date.day());
  if(date.isValid()) tcp_send(s,buf,strlen(buf));
}

void MyQDateEdit::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QDateEdit::mousePressEvent(event);
}

void MyQDateEdit::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QDateEdit::mouseReleaseEvent(event);
}

void MyQDateEdit::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QDateEdit::enterEvent(event);
}

void MyQDateEdit::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QDateEdit::leaveEvent(event);
}

MyQTimeEdit::MyQTimeEdit(int *sock, int ident, QWidget *parent, const char *name)
            :QTimeEdit(parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(timeChanged(const QTime &)), SLOT(slotValueChanged(const QTime &)));
}

MyQTimeEdit::~MyQTimeEdit()
{
}

void MyQTimeEdit::slotValueChanged(const QTime &time)
{
char buf[80];

  sprintf(buf,"text(%d,\"%d.%d.%d.%d\")\n", id, time.hour(), time.minute(), time.second(), time.msec());
  if(time.isValid()) tcp_send(s,buf,strlen(buf));
}

void MyQTimeEdit::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QTimeEdit::mousePressEvent(event);
}

void MyQTimeEdit::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QTimeEdit::mouseReleaseEvent(event);
}

void MyQTimeEdit::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QTimeEdit::enterEvent(event);
}

void MyQTimeEdit::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QTimeEdit::leaveEvent(event);
}

MyQDateTimeEdit::MyQDateTimeEdit(int *sock, int ident, QWidget *parent, const char *name)
                :QDateTimeEdit(parent)
{
  s = sock;
  id = ident;
  if(name != NULL) setObjectName(name);
  connect(this, SIGNAL(dateTimeChanged(const QDateTime &)), SLOT(slotValueChanged(const QDateTime &)));
}

MyQDateTimeEdit::~MyQDateTimeEdit()
{
}

void MyQDateTimeEdit::slotValueChanged(const QDateTime &date_time)
{
char buf[200];

  sprintf(buf,"text(%d,\"%d:%d:%d-%d.%d.%d.%d\")\n", id, date_time.date().year(),
                                                         date_time.date().month(),
                                                         date_time.date().day(),
                                                         date_time.time().hour(),
                                                         date_time.time().minute(),
                                                         date_time.time().second(),
                                                         date_time.time().msec());
  if(date_time.isValid()) tcp_send(s,buf,strlen(buf));
}

void MyQDateTimeEdit::mousePressEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonPressed(%d) -xy=%d,%d\n",id, event->x(), event->y());
  tcp_send(s,buf,strlen(buf));
  QDateTimeEdit::mousePressEvent(event);
}

void MyQDateTimeEdit::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[80];

  if(event == NULL) return;
  sprintf(buf,"QPushButtonReleased(%d) -xy=%d,%d\n",id, event->x(), event->y());
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QDateTimeEdit::mouseReleaseEvent(event);
}

void MyQDateTimeEdit::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QDateTimeEdit::enterEvent(event);
}

void MyQDateTimeEdit::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QDateTimeEdit::leaveEvent(event);
}

MyQDockWidget::MyQDockWidget(QString &title, int *sock, int ident, int dockID, QWidget *parent, const char *name)
             :QDockWidget(title, parent)
{
  s = sock;
  id = ident;
  dock_id = dockID;
  w = h = 400;
  connect(this, SIGNAL(topLevelChanged(bool)), SLOT(slotTopLevelChanged(bool)));
  if(name != NULL) setObjectName(name);
}

MyQDockWidget::~MyQDockWidget()
{
}

void MyQDockWidget::slotTopLevelChanged(bool toplevel)
{
  if(toplevel)
  {
    resize(w,h);
  }
}

void mySetForegroundColor(QWidget *w, int type, int r, int g, int b)
{
  if(w == NULL) return;
  if(r==-1 && g==-1 && b==-1)
  {
    //w->unsetPalette();
    QPalette palette;
    w->setPalette(palette);
    return;
  }
  QPalette palette = w->palette();
  if(type == TQPushButton)
  {
    palette.setColor(QPalette::ButtonText,QColor(r,g,b));
    ((MyQPushButton *)w)->setPalette(palette);
  }
  else
  {
    palette.setColor(QPalette::WindowText,QColor(r,g,b));
    w->setPalette(palette);
  }
}

void mySetBackgroundColor(QWidget *w, int type, int r, int g, int b)
{
  if(opt.arg_debug) printf("mySetBackgroundColor: type=%d r=%d g=%d b=%d\n",type,r,g,b);
  if(w == NULL) return;
  if(r==-1 && g==-1 && b==-1)
  {
    //w->unsetPalette();
    QPalette palette;
    w->setPalette(palette);
    return;
  }
  QPalette palette = w->palette();
  if(type == TQPushButton ||
     type == TQRadio      ||
     type == TQCheck      )
  {
    //palette.setColor(QPalette::Button,QColor(r,g,b));
#ifdef PVWIN32
    w->setStyle(new QWindowsStyle);
#endif
    w->setAutoFillBackground(false);
    QBrush brush(QColor(r,g,b,255));
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active,   QPalette::Button, brush);
    palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
    palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
    w->setPalette(palette);
  }
  else if(type == TQMultiLineEdit ||
          type == TQLineEdit      ||
          type == TQTextBrowser   )
  {
#ifdef PVWIN32
    w->setStyle(new QWindowsStyle);
#endif
    w->setAutoFillBackground(false);
    QBrush brush(QColor(r,g,b,255));
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active,   QPalette::Base, brush);
    palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
    palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
    w->setPalette(palette);
    //w->setAutoFillBackground(true);
    //palette.setColor(QPalette::Base,QColor(r,g,b));
    //w->setPalette(palette);
  }
  else if(type == TQDraw)
  {
    ((QDrawWidget *)w)->setBackgroundColor(r,g,b);
  }
  else
  {
    w->setAutoFillBackground(true);
    palette.setColor(QPalette::Window,QColor(r,g,b));
    w->setPalette(palette);
  }
}