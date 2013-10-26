#include "exportdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QPixmap>
#include <QFile>
#include <QMessageBox>
#include <QApplication>

#include "gl2ps.h"

ExportDialog::ExportDialog(GLWidget *widget, QWidget *parent) : QDialog(parent),glWidget(widget){

  setWindowTitle(tr("Export"));

  bar=new QProgressBar;
  bar->setValue(bar->minimum());

  makeTimeGroupBox();
  makeSizeGroupBox();
  makeExportGroupBox();
  buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(exportScene()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout *layout=new QVBoxLayout;
  layout->addWidget(timeOutGb);
  layout->addWidget(sizeOutGb);
  layout->addWidget(exportFileGb);
  layout->addWidget(bar);
  layout->addWidget(buttonBox);
  setLayout(layout);
  setFixedSize(sizeHint());
}

void ExportDialog::makeTimeGroupBox(){
  timeOutGb=new QGroupBox("Time Output");
  radioSingle=new QRadioButton(tr("Single"));
  radioAll=new QRadioButton(tr("All frames:  0  to  "));
  radioRange=new QRadioButton(tr("Range:"));
  minBox=new QSpinBox;
  maxBox=new QSpinBox;
  QLabel *toLb=new QLabel(tr(" to "));
  QButtonGroup buttonGroup;
  buttonGroup.addButton(radioSingle);
  buttonGroup.addButton(radioAll);
  buttonGroup.addButton(radioRange);
  radioSingle->setChecked(true);
  QHBoxLayout *hLayout=new QHBoxLayout;
  hLayout->addWidget(radioRange);
  hLayout->addWidget(minBox);
  hLayout->addWidget(toLb);
  hLayout->addWidget(maxBox);
  hLayout->addStretch(1);
  QVBoxLayout *vLayout=new QVBoxLayout;
  vLayout->addWidget(radioSingle);
  vLayout->addWidget(radioAll);
  vLayout->addLayout(hLayout);
  timeOutGb->setLayout(vLayout);
}

void ExportDialog::makeSizeGroupBox(){
  QString path=qApp->arguments()[0];
  int index;
  if((index=path.lastIndexOf("\\"))>-1){
    path.truncate(index+1);
    path+=QString("images\\");
  }
  else if((index=path.lastIndexOf("/"))>-1){
    path.truncate(index+1);
    path+=QString("images/");
  }
  else{
    #ifdef ROOTDIR
    path=QString(ROOTDIR);
    if((index=path.lastIndexOf("\\"))>-1){
      if(index<path.size()-1) path+=QString("\\");
      path+=QString("images\\");
    }
    else if((index=path.lastIndexOf("/"))>-1){
      if(index<path.size()-1) path+=QString("/");
      path+=QString("images/");
    }
    else
    #endif
    path=QString("./");
  }
  sizeOutGb=new QGroupBox("Output Size");
  QDoubleValidator *vd=new QDoubleValidator(this);
  QIntValidator *vi=new QIntValidator(this);
  vd->setBottom(1e-2);
  vi->setBottom(100);
  widthLe=new QLineEdit;
  widthLe->setValidator(vi);
  widthLe->setFixedWidth(50);
  widthLe->setAlignment(Qt::AlignRight);
  QObject::connect(widthLe,SIGNAL(editingFinished()),this,SLOT(widthChanged()));
  heightLe=new QLineEdit;
  heightLe->setValidator(vi);
  heightLe->setFixedWidth(50);
  heightLe->setAlignment(Qt::AlignRight);
  QObject::connect(heightLe,SIGNAL(editingFinished()),this,SLOT(heightChanged()));
  ratioLe=new QLineEdit;
  ratioLe->setValidator(vd);
  ratioLe->setFixedWidth(70);
  ratioLe->setAlignment(Qt::AlignRight);
  QObject::connect(ratioLe,SIGNAL(editingFinished()),this,SLOT(ratioChanged()));
  lockedIcon=QIcon(path+QString("locked.png"));
  unlockedIcon=QIcon(path+QString("unlocked.png"));
  lockBt=new QPushButton;
  lockBt->setCheckable(true);
  lockBt->setChecked(false);
  lockBt->setMaximumSize(20,20);
  lockBt->setMinimumSize(20,20);
  lockBt->setIcon(unlockedIcon);
  QObject::connect(lockBt,SIGNAL(toggled(bool)),this,SLOT(updateIcon(bool)));
  QPushButton *but320x240=new QPushButton(QString("320 x 240"));
  QPushButton *but640x480=new QPushButton(QString("640 x 480"));
  QPushButton *but800x600=new QPushButton(QString("800 x 600"));
  QObject::connect(but320x240,SIGNAL(pressed()),this,SLOT(set320x240()));
  QObject::connect(but640x480,SIGNAL(pressed()),this,SLOT(set640x480()));
  QObject::connect(but800x600,SIGNAL(pressed()),this,SLOT(set800x600()));
  QLabel *widthLb=new QLabel(tr("Width"));
  QLabel *heightLb=new QLabel(tr("Height"));
  QLabel *ratioLb=new QLabel(tr("Image Aspect"));
  QGridLayout *gLayout=new QGridLayout;
  gLayout->addWidget(widthLb,0,0);
  gLayout->addWidget(widthLe,0,1);
  gLayout->addWidget(but320x240,0,3);
  gLayout->addWidget(but640x480,0,4);
  gLayout->addWidget(heightLb,1,0);
  gLayout->addWidget(heightLe,1,1);
  gLayout->addWidget(but800x600,1,3);
  gLayout->setColumnStretch(2,2);
  gLayout->setColumnStretch(5,1);
  QHBoxLayout *hLayout=new QHBoxLayout;
  hLayout->addWidget(ratioLb);
  hLayout->addWidget(ratioLe);
  hLayout->addWidget(lockBt);
  hLayout->addStretch(1);
  QVBoxLayout *vLayout=new QVBoxLayout;
  vLayout->addLayout(gLayout);
  vLayout->addLayout(hLayout);
  sizeOutGb->setLayout(vLayout);
}

void ExportDialog::makeExportGroupBox(){
  exportFileGb=new QGroupBox("File");
  baseNLe=new QLineEdit;
  baseNLe->setMinimumWidth(200);
  browseBt=new QPushButton(tr("Browse..."));
  QObject::connect(browseBt,SIGNAL(pressed()),this,SLOT(getBaseName()));
  QLabel *baseNLb=new QLabel(tr("Base Name"));
  QHBoxLayout *hLayout=new QHBoxLayout;
  hLayout->addWidget(baseNLb);
  hLayout->addWidget(baseNLe);
  hLayout->addWidget(browseBt);
  exportFileGb->setLayout(hLayout);
}

void ExportDialog::updateIcon(bool toggle){
  if(toggle){
    lockBt->setIcon(lockedIcon);
    ratioLe->setEnabled(false);
  }
  else{
    lockBt->setIcon(unlockedIcon);
    ratioLe->setEnabled(true);
  }
}

void ExportDialog::updateDialog(){
  widthLe->setText(QString("%1").arg(glWidget->width()));
  heightLe->setText(QString("%1").arg(glWidget->height()));
  ratioLe->setText(QString("%1").arg((double)(glWidget->width())/glWidget->height()));
  bar->setValue(bar->minimum());
  if(glWidget->particles!=NULL){
    radioAll->setText(tr("All frames:  0  to  %1").arg(glWidget->particles->nbSteps-1));
    minBox->setMaximum(glWidget->particles->nbSteps-1);
    maxBox->setMaximum(glWidget->particles->nbSteps-1);
    maxBox->setValue(glWidget->particles->nbSteps-1);
  }
  else{
    radioAll->setText(tr("All frames:  0  to  0"));
    minBox->setMaximum(0);
    maxBox->setMaximum(0);
    maxBox->setValue(0);
  }
}

void ExportDialog::widthChanged(){
  int width=widthLe->text().toInt();
  int height=heightLe->text().toInt();
  float ratio=ratioLe->text().toFloat();
  if(lockBt->isChecked())
    heightLe->setText(QString("%1").arg(round(width/ratio)));
  else
    ratioLe->setText(QString("%1").arg((float)(width)/height));
}

void ExportDialog::heightChanged(){
  int width=widthLe->text().toInt();
  int height=heightLe->text().toInt();
  float ratio=ratioLe->text().toFloat();
  if(lockBt->isChecked())
    widthLe->setText(QString("%1").arg(round(ratio*height)));
  else
    ratioLe->setText(QString("%1").arg((float)(width)/height));
}

void ExportDialog::ratioChanged(){
  int width=widthLe->text().toInt();
  float ratio=ratioLe->text().toFloat();
  heightLe->setText(QString("%1").arg(round(width/ratio)));
}

void ExportDialog::getBaseName(){
  QString fileName = QFileDialog::getSaveFileName(
      this,QString(),QString(),"PNG (*.png);;Bitmap (*.bmp);;PS (*.ps);;EPS (*.eps);;PDF (*.pdf)");
  baseNLe->setText(fileName);
}

void ExportDialog::exportScene(){
  QString ext[]={".bmp",".png",".ps",".eps",".pdf"};
  int extLength[]={4,4,3,4,4};
  char *format[]={(char*)("BMP"),(char*)("PNG")};
  GLint format_GL2PS[]={GL2PS_PS,GL2PS_EPS,GL2PS_PDF};
  int k,size,step;
  int numFormat=5,numWithQt=2;

  size=baseNLe->text().size();
  if(!baseNLe->text().isEmpty()){
    for(k=0;k<numFormat;k++){
      if(size>extLength[k] && baseNLe->text().endsWith(ext[k]))
        break;
    }
    if(k<numFormat){
      int min=0,max=0;
      int width=widthLe->text().toInt();
      int height=heightLe->text().toInt();
      int glWidth,glHeight;
      QPixmap pixmap;
      QString fileName;
      QString baseName=baseNLe->text();
      baseName.truncate(size-extLength[k]);
      glWidth=glWidget->width();
      glHeight=glWidget->height();
      glWidget->resizegl(width,height);
      if(radioSingle->isChecked())
        min=max=glWidget->getStep();
      else if(radioAll->isChecked()){
        min=0;
        max=glWidget->particles->nbSteps-1;
      }
      else if(radioRange->isChecked()){
        min=minBox->value();
        max=maxBox->value();
      }
      bar->setRange(min,max);
      if(min==max)
        bar->setMinimum(min-1);
      if(k<numWithQt){
        glWidget->enableGlList(false);
        QFile file("");
        for(step=min;step<=max;step++){
          bar->setValue(step);
          fileName=baseName;
          fileName.append(QString("_%1").arg(step,4,10,QChar('0')));
          fileName.append(ext[k]);
          glWidget->goToStep(step);
          pixmap=glWidget->renderPixmap(width,height);
          file.setFileName(fileName);
          file.open(QFile::WriteOnly);
          pixmap.save(&file,format[k]);
          file.close();
        }
        glWidget->enableGlList(true);
      }
      else{ // with GL2PS
        FILE *pFile;
        char fileN[256];
        int i,state,buffsize;
        glShadeModel(GL_FLAT);
        for(step=min;step<=max;step++){
          state=GL2PS_OVERFLOW;
          buffsize=0;
          bar->setValue(step);
          fileName=baseName;
          fileName.append(QString("_%1").arg(step,4,10,QChar('0')));
          fileName.append(ext[k]);
          for(i=0;i<fileName.size();i++)
            fileN[i]=fileName[i].toAscii();
          fileN[i]='\0';
          pFile=fopen(fileN,"wb");
          while(state == GL2PS_OVERFLOW){
            buffsize += 1024*1024;
            gl2psBeginPage("Export", "MoleculeV", NULL, format_GL2PS[k-numWithQt], GL2PS_SIMPLE_SORT,
                           GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT | GL2PS_OCCLUSION_CULL,
                           GL_RGBA, 0, NULL, 10, 10, 10, buffsize, pFile, fileN);
            if(step==glWidget->getStep())
              glWidget->updateGL();
            else
              glWidget->goToStep(step);
            state=gl2psEndPage();
          }
          fclose(pFile);
        }
        glShadeModel(GL_SMOOTH);
      }
      glWidget->resizegl(glWidth,glHeight);
      QMessageBox::information(this,"Export","Done!");
      accept();
    }
  }
}


void ExportDialog::set320x240(){
  widthLe->setText(QString("%1").arg(320));
  heightLe->setText(QString("%1").arg(240));
  ratioLe->setText(QString("%1").arg(4./3.));
}

void ExportDialog::set640x480(){
  widthLe->setText(QString("%1").arg(640));
  heightLe->setText(QString("%1").arg(480));
  ratioLe->setText(QString("%1").arg(4./3.));
}

void ExportDialog::set800x600(){
  widthLe->setText(QString("%1").arg(800));
  heightLe->setText(QString("%1").arg(600));
  ratioLe->setText(QString("%1").arg(4./3.));
}
