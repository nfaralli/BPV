#include "blankingdialog.h"
#include <QDoubleValidator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

BlankingDialog::BlankingDialog(QWidget *parent) : QDialog(parent){
	setWindowTitle(tr("Blanking"));
	
	int i;
	activeCB		= new QCheckBox;
	selBlankCB	= new QComboBox;
	allTypesRB	= new QRadioButton("all types");
	oneTypeRB		= new QRadioButton("one type:");
	typeNameCB	= new QComboBox;
	varNameCB		= new QComboBox;
	operatorCB	= new QComboBox;
	valueLE			= new QLineEdit;
	closeBB			=	new QDialogButtonBox(QDialogButtonBox::Close);
	for(i=0;i<NB_BLANK;i++)
		selBlankCB->addItem(QString("Blanking #%1").arg(i+1));
	varNameCB->setMinimumWidth(100);
	operatorCB->addItem("<");
	operatorCB->addItem("<=");
	operatorCB->addItem("==");
	operatorCB->addItem(">=");
	operatorCB->addItem(">");
	operatorCB->addItem("!=");
  QDoubleValidator *vd=new QDoubleValidator(this);
	valueLE->setValidator(vd);
	valueLE->setMinimumWidth(60);
	valueLE->setMaximumWidth(60);
	
	connect(closeBB,SIGNAL(rejected()),this,SLOT(reject()));
	connect(activeCB,SIGNAL(clicked(bool)),allTypesRB,SLOT(setEnabled(bool)));
	connect(activeCB,SIGNAL(clicked(bool)),oneTypeRB,SLOT(setEnabled(bool)));
	connect(activeCB,SIGNAL(clicked(bool)),typeNameCB,SLOT(setEnabled(bool)));
	connect(activeCB,SIGNAL(clicked(bool)),varNameCB,SLOT(setEnabled(bool)));
	connect(activeCB,SIGNAL(clicked(bool)),operatorCB,SLOT(setEnabled(bool)));
	connect(activeCB,SIGNAL(clicked(bool)),valueLE,SLOT(setEnabled(bool)));
	connect(selBlankCB,SIGNAL(currentIndexChanged(int)),this,SLOT(setOptions(int)));
	connect(activeCB,SIGNAL(clicked(bool)),this,SLOT(changeActive(bool)));
	connect(allTypesRB,SIGNAL(toggled (bool)),this,SLOT(changeAllTypes(bool)));
	connect(typeNameCB,SIGNAL(currentIndexChanged(int)),this,SLOT(changeTypeIndex(int)));
	connect(varNameCB,SIGNAL(currentIndexChanged(int)),this,SLOT(changeVarIndex(int)));
	connect(operatorCB,SIGNAL(currentIndexChanged(int)),this,SLOT(changeOpIndex(int)));
	connect(valueLE,SIGNAL(editingFinished()),this,SLOT(changeValue()));
	
	QHBoxLayout *selBlankLayout=new QHBoxLayout;
	selBlankLayout->addWidget(activeCB);
	selBlankLayout->addWidget(selBlankCB);
	selBlankLayout->setStretchFactor(selBlankCB,1);
	QGridLayout *selTypeLayout=new QGridLayout;
	selTypeLayout->addWidget(allTypesRB,0,0);
	selTypeLayout->addWidget(oneTypeRB,1,0);
	selTypeLayout->addWidget(typeNameCB,1,1);
	selTypeLayout->setColumnStretch(1,1);
	QHBoxLayout *setEquationLayout=new QHBoxLayout;
	setEquationLayout->addWidget(varNameCB);
	setEquationLayout->addWidget(operatorCB);
	setEquationLayout->addWidget(valueLE);
	setEquationLayout->setStretchFactor(varNameCB,1);
	QVBoxLayout *dialogLayout=new QVBoxLayout;
	dialogLayout->addLayout(selBlankLayout);
	dialogLayout->addLayout(selTypeLayout);
	dialogLayout->addLayout(setEquationLayout);
	dialogLayout->addWidget(closeBB);
	setLayout(dialogLayout);
	
	allowEmit=true;
	emptyDialog();
  setFixedSize(sizeHint());
}

void BlankingDialog::setDialog(Particles *particules){
	int i;
	if(particules!=NULL){
		allowEmit=false;
		emptyDialog();
		for(i=0;i<particules->nbTypes;i++)
			typeNameCB->addItem(particules->pSpec[i].name);
		varNameCB->addItem("time [s]");
		for(i=0;i<3+particules->nbVariables;i++)
			varNameCB->addItem(particules->varName[i]);
		allowEmit=true;
	}
}

void BlankingDialog::emptyDialog(){
	while(typeNameCB->count()>0)
		typeNameCB->removeItem(0);
	while(varNameCB->count()>0)
		varNameCB->removeItem(0);
	for(int i=0;i<NB_BLANK;i++){
		blanks[i].active=false;
		blanks[i].allTypes=true;
		blanks[i].typeIndex=0;
		blanks[i].varIndex=0;
		blanks[i].opIndex=0;
		blanks[i].value=0;
	}
	activeCB->setChecked(false);
	selBlankCB->setCurrentIndex(0);
	allTypesRB->setChecked(true);
	operatorCB->setCurrentIndex(0);
	valueLE->setText(QString("%1").arg(blanks[0].value));
	allTypesRB->setEnabled(false);
	oneTypeRB->setEnabled(false);
	typeNameCB->setEnabled(false);
	varNameCB->setEnabled(false);
	operatorCB->setEnabled(false);
	valueLE->setEnabled(false);
}

void BlankingDialog::setOptions(int index){
	allowEmit=false;
	activeCB->setChecked(blanks[index].active);
	allTypesRB->setEnabled(blanks[index].active);
	oneTypeRB->setEnabled(blanks[index].active);
	typeNameCB->setEnabled(blanks[index].active);
	varNameCB->setEnabled(blanks[index].active);
	operatorCB->setEnabled(blanks[index].active);
	valueLE->setEnabled(blanks[index].active);
	if(blanks[index].allTypes)
		allTypesRB->setChecked(true);
	else
		oneTypeRB->setChecked(true);
	typeNameCB->setCurrentIndex(blanks[index].typeIndex);
	varNameCB->setCurrentIndex(blanks[index].varIndex);
	operatorCB->setCurrentIndex(blanks[index].opIndex);
	valueLE->setText(QString("%1").arg(blanks[index].value));
	allowEmit=true;
}

void BlankingDialog::changeActive(bool checked){
	blanks[selBlankCB->currentIndex()].active=checked;
	if(allowEmit)
		emit blankChanged(selBlankCB->currentIndex(),blanks[selBlankCB->currentIndex()]);
}

void BlankingDialog::changeAllTypes(bool checked){
	blanks[selBlankCB->currentIndex()].allTypes=checked;
	if(allowEmit)
		emit blankChanged(selBlankCB->currentIndex(),blanks[selBlankCB->currentIndex()]);
}

void BlankingDialog::changeTypeIndex(int index){
	blanks[selBlankCB->currentIndex()].typeIndex=index;
	if(allowEmit)
		emit blankChanged(selBlankCB->currentIndex(),blanks[selBlankCB->currentIndex()]);
}

void BlankingDialog::changeVarIndex(int index){
	blanks[selBlankCB->currentIndex()].varIndex=index;
	if(allowEmit)
		emit blankChanged(selBlankCB->currentIndex(),blanks[selBlankCB->currentIndex()]);
}

void BlankingDialog::changeOpIndex(int index){
	blanks[selBlankCB->currentIndex()].opIndex=index;
	if(allowEmit)
		emit blankChanged(selBlankCB->currentIndex(),blanks[selBlankCB->currentIndex()]);
}

void BlankingDialog::changeValue(){
	blanks[selBlankCB->currentIndex()].value=valueLE->text().toFloat();
	if(allowEmit)
		emit blankChanged(selBlankCB->currentIndex(),blanks[selBlankCB->currentIndex()]);
}
