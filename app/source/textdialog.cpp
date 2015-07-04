#include "textdialog.h"
#include "fonts.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QIcon>
#include <QColorDialog>
#include <QDir>
#include <QStringList>


TextDialog::TextDialog(QWidget *parent) :
  QDialog(parent){
  setWindowTitle(tr("Text"));

  currentFont="";

  textColor = Qt::white;
  showCB =  new QCheckBox("Show");
  showCB->setChecked(true);
  colorPB = new QPushButton();
  colorPB->setMinimumWidth(50);
  fontCB = new QComboBox;
  sizeCB = new QComboBox;
  closeBB = new QDialogButtonBox(QDialogButtonBox::Close);
  findFonts();
  enableFontChangedSig=true;

  connect(closeBB, SIGNAL(rejected()), this, SLOT(reject()));
  connect(showCB, SIGNAL(clicked(bool)), this, SIGNAL(showTextChanged(bool)));
  connect(colorPB, SIGNAL(pressed()), this, SLOT(changeColor()));
  connect(fontCB,SIGNAL(currentIndexChanged(int)),this,SLOT(updateSizeCB()));
  connect(sizeCB,SIGNAL(currentIndexChanged(int)),this,SLOT(updateCurrentFont()));

  QVBoxLayout *vLayout = new QVBoxLayout;
  vLayout->addWidget(showCB);
  QGridLayout *gLayout = new QGridLayout;
  QLabel *label;
  label=new QLabel("Font");
  gLayout->addWidget(label,0,0);
  gLayout->setAlignment(label,Qt::AlignHCenter);
  label=new QLabel("Size");
  gLayout->addWidget(label,0,1);
  gLayout->setAlignment(label,Qt::AlignHCenter);
  label=new QLabel("Color");
  gLayout->addWidget(label,0,2);
  gLayout->setAlignment(label,Qt::AlignHCenter);
  gLayout->addWidget(fontCB,1,0);
  gLayout->setAlignment(fontCB,Qt::AlignHCenter);
  gLayout->addWidget(sizeCB,1,1);
  gLayout->setAlignment(sizeCB,Qt::AlignHCenter);
  gLayout->addWidget(colorPB,1,2);
  gLayout->setAlignment(colorPB,Qt::AlignHCenter);
  vLayout->addLayout(gLayout);
  vLayout->addWidget(closeBB);
  setLayout(vLayout);

  QPixmap colorPixmap(20,20);
  colorPixmap.fill(textColor);
  QIcon colorIcon(colorPixmap);
  colorPB->setIcon(colorIcon);
  setFixedSize(sizeHint());
}

void TextDialog::findFonts(){
  QStringList bmpFiles, txtFiles;
  int  i, index, indexList, fontSize;
  bool ok;

  for(i=0; i<(int)(sizeof(FONTS_FILE_NAMES)/sizeof(*FONTS_FILE_NAMES)); i++){
    if(QString(FONTS_FILE_NAMES[i]).contains(".bmp"))
      bmpFiles.append(FONTS_FILE_NAMES[i]);
    else if(QString(FONTS_FILE_NAMES[i]).contains(".txt"))
      txtFiles.append(FONTS_FILE_NAMES[i]);
  }
  for(i=0;i<bmpFiles.size();i++){
    QString bmpFile=bmpFiles.at(i);
    bmpFile.chop(4);//removes ".bmp"
    if(!txtFiles.contains(bmpFile+".txt")) //check if corresponding .txt file exists
      continue;
    index=bmpFile.lastIndexOf('_');
    if(index==-1)
      continue;
    fontSize=bmpFile.mid(index+1).toInt(&ok);
    if(!ok)
      continue;
    indexList=fontList.indexOf(bmpFile.left(index)); //check if font already exists
    if(indexList==-1){//new font
      fontList.append(bmpFile.left(index));
      sizeList.append(QList<int>());
      sizeList[sizeList.size()-1].append(fontSize);
      continue;
    }
    sizeList[indexList].append(fontSize);
  }
  //found all the fonts, update the ComboBoxes now
  enableFontChangedSig=false;
  while(fontCB->count())
    fontCB->removeItem(0);
  fontCB->addItems(fontList);
  if(fontList.contains(DEFAULT_FONT))
    fontCB->setCurrentIndex(fontList.indexOf(DEFAULT_FONT));
  else
    fontCB->setCurrentIndex(0);
  while(sizeCB->count())
    sizeCB->removeItem(0);
  for(i=0;i<sizeList[fontCB->currentIndex()].size();i++)
    sizeCB->addItem(QString("%1").arg(sizeList[fontCB->currentIndex()][i]));
  if(sizeList[fontCB->currentIndex()].contains(DEFAULT_FONT_SIZE))
    sizeCB->setCurrentIndex(sizeList[fontCB->currentIndex()].indexOf(DEFAULT_FONT_SIZE));
  else
    sizeCB->setCurrentIndex(0);
  updateCurrentFont();
  enableFontChangedSig=true;
}

void TextDialog::changeColor(){
  QColor newColor = QColorDialog::getColor(textColor);
  if(newColor.isValid()){
    textColor=newColor;
    QPixmap colorPixmap(20,20);
    colorPixmap.fill(textColor);
    QIcon colorIcon(colorPixmap);
    colorPB->setIcon(colorIcon);
    emit colorChanged(textColor);
  }
}

// returns the name of the .bmp file containing the font
QString TextDialog::getFont(){
  return currentFont;
}

void TextDialog::updateSizeCB(){
  if(!enableFontChangedSig)
    return;
  while(sizeCB->count())
    sizeCB->removeItem(0);
  for(int i=0;i<sizeList[fontCB->currentIndex()].size();i++)
    sizeCB->addItem(QString("%1").arg(sizeList[fontCB->currentIndex()][i]));
  sizeCB->setCurrentIndex(0);
  updateCurrentFont();
}

void TextDialog::updateCurrentFont(){
  currentFont=fontCB->currentText()+"_"+sizeCB->currentText()+".bmp";
  if(enableFontChangedSig)
    emit fontChanged(currentFont.toStdString().data());
}
