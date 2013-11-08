#ifndef __EXPORTWIDGET_H__
#define __EXPORTWIDGET_H__

#include <QDialog>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QPushButton>
#include <QIcon>
#include <QLineEdit>
#include <QProgressBar>

#include "glwidget.h"

class ExportDialog : public QDialog{

  Q_OBJECT

public:
  ExportDialog(GLWidget *widget, QWidget *parent=0);

public slots:
  void updateDialog();

private slots:
  void updateIcon(bool);
  void getBaseName();
  void widthChanged();
  void heightChanged();
  void ratioChanged();
  void exportScene();
  void set320x240();
  void set640x480();
  void set800x600();

private:
  void makeTimeGroupBox();
  void makeSizeGroupBox();
  void makeExportGroupBox();

  QGroupBox        *timeOutGb;
  QGroupBox        *sizeOutGb;
  QGroupBox        *exportFileGb;
  QDialogButtonBox *buttonBox;
  GLWidget         *glWidget;
  QRadioButton     *radioSingle;
  QRadioButton     *radioAll;
  QRadioButton     *radioRange;
  QSpinBox         *minBox;
  QSpinBox         *maxBox;
  QLineEdit        *widthLe;
  QLineEdit        *heightLe;
  QLineEdit        *ratioLe;
  QPushButton      *lockBt;
  QIcon            lockedIcon;
  QIcon            unlockedIcon;
  QLineEdit        *baseNLe;
  QPushButton      *browseBt;
  QProgressBar     *bar;
};

#endif
