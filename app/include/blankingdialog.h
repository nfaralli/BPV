#ifndef __BLANKINGDIALOG_H__
#define __BLANKINGDIALOG_H__

#include <QDialog>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QDialogButtonBox>

#include "particles.h"

#define NB_BLANK 10

typedef struct{
  bool   active;    // true if blank is active
  bool   allTypes;  // true if blank applies on all types
  int    typeIndex; // type index if blank applies on one type
  int    varIndex;  // variable index (=0 for time, =1-3 for coordinates)
  int    opIndex;   // operator index
  float  value;     // value used for comparison with variable
} Blank;

class BlankingDialog : public QDialog{

  Q_OBJECT

public:
  BlankingDialog(QWidget *parent=0);

public slots:
  void setDialog(Particles *particules);

signals:
  void blankChanged(int,Blank);

private:
  void emptyDialog();

private slots:
  void setOptions(int index);
  void changeActive(bool);
  void changeAllTypes(bool);
  void changeTypeIndex(int);
  void changeVarIndex(int);
  void changeOpIndex(int);
  void changeValue();

private:
  QCheckBox        *activeCB;
  QComboBox        *selBlankCB;
  QRadioButton     *allTypesRB;
  QRadioButton     *oneTypeRB;
  QComboBox        *typeNameCB;
  QComboBox        *varNameCB;
  QComboBox        *operatorCB;
  QLineEdit        *valueLE;
  QDialogButtonBox *closeBB;
  Blank            blanks[NB_BLANK];
  bool             allowEmit;
};

#endif
