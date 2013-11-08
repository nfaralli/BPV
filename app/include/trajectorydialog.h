#ifndef __TRAJECTORYDIALOG_H__
#define __TRAJECTORYDIALOG_H__

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

#include "mycolor.h"
#include "particles.h"

#define NB_TRAJECTORIES 11

typedef struct {
  bool active; // true if trajectory is active
  bool single; // true if this represent a single trajectory, false otherwise.
  int index; // particle index (starting at 0)(if single trajectory)
  int nbPrevSteps; // max number of previous steps shown for trajectory (0=infinite)
  MyColor color; // trajectory color;
} Trajectory;

class TrajectoryDialog: public QDialog {

Q_OBJECT

public:
  TrajectoryDialog(QWidget *parent = 0);
  ~TrajectoryDialog();

public slots:
  void setDialog(Particles*);

signals:
  void animTraChanged(bool);
  void trajectoryChanged(int, Trajectory);

protected slots:
  void changeAllTraBt(bool);
  void changeAllTraColor();
  void changeAllTraLength(int);
  void changeSingleTraBt(bool);
  void changeSingleTraIndex(int);
  void changeParticleIndex(int);
  void changeSingleTraColor();
  void changeSingleTraLength(int);

private:
  QCheckBox *animateCB;
  QCheckBox *activeAllTraCB;
  QCheckBox *activeSingleTraCB;
  QComboBox *selSingleTraCB;
  QLabel *allTraColorLb;
  QLabel *allTraLengthLb;
  QLabel *indexparticleLb;
  QLabel *singleTraColorLb;
  QLabel *singleTraLengthLb;
  QSpinBox *allTraLengthSB;
  QSpinBox *indexParticleSB;
  QSpinBox *singleTraLengthSB;
  QPushButton *allTraColorPB;
  QPushButton *singleTraColorPB;
  QDialogButtonBox *closeBB;
  Trajectory trajectory[NB_TRAJECTORIES];
  Particles *minMaxParticles;
  bool allowEmit;
};

#endif
