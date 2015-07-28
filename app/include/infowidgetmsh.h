#ifndef __INFOWIDGETMSH_H__
#define __INFOWIDGETMSH_H__

#include <QObject>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include "particles.h"

class InfoWidgetMsh : public QGroupBox{

  Q_OBJECT

public:
  InfoWidgetMsh(Particles *part=0,QWidget *parent=0);
  void loadParticles(Particles *part);

signals:
  void meshSpecChanged(int, MeshSpec);
  void allMeshSpecChanged(MeshSpec*);

protected slots:
  void changeActiveMesh(bool enable);
  void setMesh(int meshIndex);
  void changeShowWires(bool enable);
  void changeAllShowWires();
  void changeCullBackFace(bool enable);
  void changeAllCullBackFace();

private:
  void enableWidgets();

private:
  int         nbMeshes;
  MeshSpec    *mSpec;
  QCheckBox   *activeMeshBx;
  QComboBox   *meshBx;
  QCheckBox   *wireBx;
  QPushButton *wireBt;
  QCheckBox   *cullBackFaceBx;
  QPushButton *cullBackFaceBt;
};

#endif

