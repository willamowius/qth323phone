#include <QPainter>
#include "cvideodlg.h"
#include "qtphonedlg.h"

CVideoDlg::CVideoDlg(QtPhoneDlg *dlg)
	: QDialog(0)
{
	ui.setupUi(this);	
	installEventFilter(this);
	setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

	p_dlg = dlg;
}

CVideoDlg::~CVideoDlg()
{

}
	
void CVideoDlg::closeEvent(QCloseEvent *ev)
{
	p_dlg->slot_ShowVideoPanels(false);
}

bool CVideoDlg::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Paint) 
	{
		QPainter painter(this);

		QRect tRect = geometry();
			
		painter.fillRect(tRect, Qt::black);

		m_mutex.lock();

		if(!m_image.isNull())
			painter.drawImage(0, 0, m_image.scaled(tRect.width(), tRect.height(), Qt::KeepAspectRatio));
			//painter.drawImage(0,0, m_image);

		m_mutex.unlock();

		return true;
	}
	return QObject::eventFilter(obj, event);
}
