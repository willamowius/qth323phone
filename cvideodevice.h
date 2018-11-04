#ifndef CVIDEODEVICE_H
#define CVIDEODEVICE_H

#include <QObject>
#include <QRect>
#include "global.h"
#include <ptlib/video.h>

class CMyPhoneEndPoint;
class CVideoDlg;

class CVideoOutputDevice: public QObject, public PVideoOutputDeviceRGB
{
	Q_OBJECT

public:
	CVideoOutputDevice(CMyPhoneEndPoint* pendpoint,
                bool fFlipVertically = FALSE,
                bool fFlipHorizontally = FALSE,
                bool isLocal = FALSE,
                bool doLocalVideoPnP = FALSE);

	~CVideoOutputDevice();

	virtual PBoolean Open(
		const PString & deviceName,   /// Device name to open
		PBoolean startImmediate = TRUE    /// Immediately start device
		);

	virtual PBoolean IsOpen();

	virtual PBoolean Close();

#if PTLIB_MAJOR<=2 && PTLIB_MINOR<10
	virtual PStringList GetDeviceNames() const;
#else
	virtual PStringArray GetDeviceNames() const;
#endif

	// Resizing frames
	PBoolean SetFrameSize(
		unsigned width,   /// New width of frame
		unsigned height   /// New height of frame
		);

	PBoolean SetColourFormat(
		const PString & colourFormat // New color format for device.
		);
	PBoolean FrameComplete();


protected:
	CVideoDlg*		m_videoDlg;

	//QRect           m_rect;
	bool			m_fVertFlip;
	bool			m_fHorzFlip;
	bool			m_isLocal;
	bool			m_doLocalVideoPnP;
	int				m_x;
	int				m_y;
	int				m_w;
	int				m_h;
	bool			m_bIsInitialized;

signals:
   void signal_ShowVideoPanels(bool);
   void signal_UpdateVideoDlg();
};

#endif // CVIDEODEVICE_H
