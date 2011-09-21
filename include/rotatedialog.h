#ifndef __ROTATEDIALOG_H__
#define __ROTATEDIALOG_H__

#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>

class RotateDialog : public QDialog{
	
	Q_OBJECT
	
public:
	RotateDialog(QWidget *parent=0);

signals:
	void centerChanged(float xCenter, float yCenter, float zCenter);
	void rotAnglesChanged(float xRot, float yRot, float zRot);
	void resetCenter();
	
public slots:
	void setCenter(float newX,float newY, float newZ);
	void setRotAngles(float newXRot, float newYRot,float newZRot);
	
private slots:
	void setXYView();
	void setYZView();
	void setXZView();
	void setRot();
	void setCenterSc();
	
private:
	QGroupBox 				*centerGB;
	QGroupBox 				*anglesGB;
	QGroupBox 				*viewsGB;
	QPushButton 			*resetPB;
	QPushButton 			*xyViewPB;
	QPushButton 			*yzViewPB;
	QPushButton 			*xzViewPB;
	QDialogButtonBox 	*buttonBox;
	QLineEdit					*xCenterLe;
	QLineEdit					*yCenterLe;
	QLineEdit					*zCenterLe;
	QLineEdit					*xRotLe;
	QLineEdit					*yRotLe;
	QLineEdit					*zRotLe;
};


#endif
