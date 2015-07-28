#include "mymainwin.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QScrollBar>
#include <QScrollArea>

MyMainWin::MyMainWin(){

  float alpha=0.7;
  int   xScreen=QApplication::desktop()->width();
  int   yScreen=QApplication::desktop()->height();
  //biggest aspect ratio=(4/3)^3. if bigger, most likely a dual screen
  if(((float)xScreen)/yScreen > (64./27.*1.1))
    xScreen/=2;
  if(((float)xScreen)/yScreen < (4./3./1.1))
    yScreen/=2;
  resize((int)(xScreen*alpha),(int)(yScreen*alpha));
  move((int)(xScreen*(1-alpha)/2),(int)(yScreen*(1-alpha)/2));
  setWindowTitle(tr("bpv"));

  particles=NULL;
  geometry=NULL;
  myWidget=new MyWidget(this);
  setCentralWidget(myWidget);

  QMenuBar *menu = menuBar();
  fileMenu = menu->addMenu(tr("&File"));
  openAct = new QAction(tr("&Open..."), this);
  openAct->setShortcut(tr("Ctrl+O"));
  QObject::connect(openAct,SIGNAL(triggered()),this,SLOT(open()));
  exportAct = new QAction(tr("&Export..."),this);
  exportAct->setShortcut(tr("Ctrl+E"));
  exportAct->setEnabled(false);
  QObject::connect(exportAct,SIGNAL(triggered()),myWidget,SLOT(exportScene()));
  QObject::connect(myWidget,SIGNAL(particlesChanged(bool)),exportAct,SLOT(setEnabled(bool)));
  QObject::connect(myWidget,SIGNAL(geometryChanged(bool)),exportAct,SLOT(setEnabled(bool)));
  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  QObject::connect(exitAct,SIGNAL(triggered()),qApp,SLOT(quit()));
  fileMenu->addAction(openAct);
  fileMenu->addAction(exportAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  menu->addSeparator();
  viewMenu = menu->addMenu(tr("&View"));
  zoomFitAct= new QAction(tr("&Zoom Fit"),this);
  zoomFitAct->setShortcut(tr("Ctrl+F"));
  zoomFitAct->setEnabled(false);
  QObject::connect(zoomFitAct,SIGNAL(triggered()),myWidget,SLOT(zoomFit()));
  QObject::connect(myWidget,SIGNAL(particlesChanged(bool)),zoomFitAct,SLOT(setEnabled(bool)));
  QObject::connect(myWidget,SIGNAL(geometryChanged(bool)),zoomFitAct,SLOT(setEnabled(bool)));
  centerAct = new QAction(tr("&Center"),this);
  centerAct->setShortcut(tr("Ctrl+C"));
  centerAct->setEnabled(false);
  QObject::connect(centerAct,SIGNAL(triggered()),myWidget,SLOT(center()));
  QObject::connect(myWidget,SIGNAL(particlesChanged(bool)),centerAct,SLOT(setEnabled(bool)));
  QObject::connect(myWidget,SIGNAL(geometryChanged(bool)),centerAct,SLOT(setEnabled(bool)));
  axisAct = new QAction(tr("&Axis"),this);
  axisAct->setShortcut(tr("Ctrl+A"));
  axisAct->setCheckable(true);
  axisAct->setChecked(true);
  QObject::connect(axisAct,SIGNAL(toggled(bool)),myWidget,SIGNAL(axisToggled(bool)));
  viewAct = new QAction(tr("&Perspective"),this);
  viewAct->setShortcut(tr("Ctrl+P"));
  viewAct->setCheckable(true);
  QObject::connect(viewAct,SIGNAL(toggled(bool)),myWidget,SIGNAL(viewToggled(bool)));
  rotateAct=new QAction(tr("&Rotate..."),this);
  rotateAct->setShortcut(tr("Ctrl+R"));
  QObject::connect(rotateAct,SIGNAL(triggered()),myWidget,SLOT(rotateScene()));
  blankingAct=new QAction(tr("&Blanking..."),this);
  blankingAct->setShortcut(tr("Ctrl+B"));
  blankingAct->setEnabled(false);
  QObject::connect(blankingAct,SIGNAL(triggered()),myWidget,SLOT(setBlanking()));
  QObject::connect(myWidget,SIGNAL(particlesChanged(bool)),blankingAct,SLOT(setEnabled(bool)));
  trajectoryAct=new QAction(tr("&Trajectory..."),this);
  trajectoryAct->setShortcut(tr("Ctrl+T"));
  trajectoryAct->setEnabled(false);
  QObject::connect(trajectoryAct,SIGNAL(triggered()),myWidget,SLOT(setTrajectory()));
  QObject::connect(myWidget,SIGNAL(particlesChanged(bool)),trajectoryAct,SLOT(setEnabled(bool)));
  textAct=new QAction(tr("Te&xt..."),this);
  textAct->setShortcut(tr("Ctrl+X"));
  QObject::connect(textAct,SIGNAL(triggered()),myWidget,SLOT(setText()));
  viewMenu->addAction(zoomFitAct);
  viewMenu->addAction(centerAct);
  viewMenu->addAction(rotateAct);
  viewMenu->addAction(blankingAct);
  viewMenu->addAction(trajectoryAct);
  viewMenu->addAction(axisAct);
  viewMenu->addAction(textAct);
  viewMenu->addAction(viewAct);

  menu->addSeparator();
  helpMenu = menu->addMenu(tr("&Help"));
  aboutAct = new QAction(tr("&About bpv..."),this);
  QObject::connect(aboutAct,SIGNAL(triggered()),this,SLOT(about()));
  aboutQtAct = new QAction(tr("About &Qt..."),this);
  QObject::connect(aboutQtAct,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);

  createDockWindows();
}

void MyMainWin::createDockWindows(){
  dockWidget = new QDockWidget(tr("Visualization"), this);
  dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

  infoBgd=new InfoWidgetBgd;
  QObject::connect(infoBgd,SIGNAL(backgroundChanged(MyColor)),
                   myWidget->glWidget,SLOT(updateBackground(MyColor)));
  infoPar=new InfoWidgetPar;
  QObject::connect(infoPar,SIGNAL(clicked(bool)),
                   myWidget->glWidget,SLOT(setEnabledParticles(bool)));
  QObject::connect(infoPar,SIGNAL(partSpecChanged(int,PartSpec)),
                   myWidget->glWidget,SLOT(setPartSpec(int,PartSpec)));
  QObject::connect(infoPar,SIGNAL(allPartSpecChanged(PartSpec*))
                   ,myWidget->glWidget,SLOT(setAllPartSpec(PartSpec*)));
  infoBnd=new InfoWidgetBnd;
  QObject::connect(infoBnd,SIGNAL(clicked(bool)),myWidget->glWidget,SLOT(setEnabledBonds(bool)));
  QObject::connect(infoBnd,SIGNAL(bondSpecChanged(BondSpec)),
                   myWidget->glWidget,SLOT(setBondSpec(BondSpec)));
  infoMsh=new InfoWidgetMsh;
  QObject::connect(infoMsh,SIGNAL(clicked(bool)),myWidget->glWidget,SLOT(setEnabledMeshes(bool)));
  QObject::connect(infoMsh,SIGNAL(meshSpecChanged(int,MeshSpec)),
                   myWidget->glWidget,SLOT(setMeshSpec(int,MeshSpec)));
  QObject::connect(infoMsh,SIGNAL(allMeshSpecChanged(MeshSpec*)),
                   myWidget->glWidget,SLOT(setAllMeshSpec(MeshSpec*)));
  infoGeo=new InfoWidgetGeo;
  QObject::connect(infoGeo,SIGNAL(clicked(bool)),myWidget->glWidget,SLOT(setEnabledGeometry(bool)));
  QObject::connect(infoGeo,SIGNAL(geometryChanged()),myWidget->glWidget,SLOT(resetGeometry()));

  setDockWidget();
  visuAct=dockWidget->toggleViewAction();
  visuAct->setText(tr("&Visualization..."));
  visuAct->setShortcut(tr("Ctrl+V"));
  visuAct->setCheckable(true);
  visuAct->setChecked(false);
  visuAct->setEnabled(false);
  QObject::connect(myWidget,SIGNAL(particlesChanged(bool)),visuAct,SLOT(setEnabled(bool)));
  QObject::connect(myWidget,SIGNAL(geometryChanged(bool)),visuAct,SLOT(setEnabled(bool)));
  viewMenu->addAction(visuAct);
  dockWidget->close();
  addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
}

void MyMainWin::setDockWidget(){
    QWidget *setup;
    QScrollArea *scrollArea;
    QVBoxLayout *vLayout=new QVBoxLayout;
    vLayout->addWidget(infoBgd);
    if(particles!=NULL){
        vLayout->addWidget(infoPar);
        if(particles->nbBonds>0){
          vLayout->addWidget(infoBnd);
        }
        if(particles->nbMeshes>0) {
          vLayout->addWidget(infoMsh);
        }
    }
    if(geometry!=NULL) {
        vLayout->addWidget(infoGeo);
    }
    vLayout->addStretch();
    setup = new QWidget(this);
    setup->setLayout(vLayout);
    setup->setMaximumWidth(vLayout->minimumSize().width());
    scrollArea = new QScrollArea(this);
    scrollArea->setWidget(setup);
    int width = vLayout->minimumSize().width()+scrollArea->verticalScrollBar()->sizeHint().width();
    // the +2 seems to be needed on Mac to get rid of the horizontal scroll bar.
    scrollArea->setMinimumWidth(width+2);
    scrollArea->setMaximumWidth(width+2);
    dockWidget->setWidget(scrollArea);
}

void MyMainWin::open(){
  QString fileName = QFileDialog::getOpenFileName(
      this,QString(),QString(),QString("Data file (*.par);;Geometry file (*.geo)"));
  open(fileName);
}

void MyMainWin::open(QString fileName){
  if(!fileName.isEmpty()){
    if(fileName.endsWith(".par")){
      Particles *part=NULL;
      part=loadParticles(fileName);
      if(part!=NULL){
        myWidget->clearParticles();
        freeParticles(particles);
        particles=part;
        myWidget->setNewParticles(particles);
        infoPar->loadParticles(particles);
        infoBnd->loadParticles(particles);
        infoMsh->loadParticles(particles);
        axisAct->setChecked(true);
        setDockWidget();
      }
    }
    else if(fileName.endsWith(".geo")){
      MyGeometry *geo;
      geo=loadGeometry(fileName);
      if(geo!=NULL){
        myWidget->clearGeometry();
        freeGeometry(geometry);
        geometry=geo;
        myWidget->setNewGeometry(geometry);
        infoGeo->loadGeometry(geometry);
        setDockWidget();
      }
    }
    else
      QMessageBox::warning(NULL,QObject::tr("Open"),QObject::tr("Unknown type."));
  }
}

void MyMainWin::about(){
  QMessageBox::about(this, tr("About bpv"),
      tr("<h3>About bpv</h3>"
      "<p>This basic particle viewer (bpv) was developped in 2008-2009 by Nicolas Faralli.</p>"
      "Report problems to <a href=\"mailto:nfaralli@asu.edu\">nfaralli@gmail.com</a>."));
}
