#ifndef __TEXTDIALOG_H__
#define __TEXTDIALOG_H__

#include <QObject>
#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QString>
#include <QStringList>
#include <QList>

#define DEFAULT_FONT "courier"
#define DEFAULT_FONT_SIZE 12

class TextDialog : public QDialog {

Q_OBJECT

public:
  TextDialog(QString fPath, QWidget *parent = 0);
  QString getFont();

signals:
  void showTextChanged(bool);
  void colorChanged(QColor);
  void fontChanged(char*);

private slots:
  void changeColor();
  void updateCurrentFont();
  void updateSizeCB();

private:
  void findFonts();

private:
  QColor             textColor;
  QCheckBox          *showCB;
  QPushButton        *colorPB;
  QComboBox          *fontCB;
  QComboBox          *sizeCB;
  QDialogButtonBox   *closeBB;
  QString            fontPath;
  QString            currentFont;
  QStringList        fontList;
  QList<QList<int> > sizeList;
  bool               enableFontChangedSig;
};

#endif
