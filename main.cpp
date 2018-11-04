#include "qtphonedlg.h"
#include <QtGui/QApplication>
#include <QTextCodec>


/*
int main(int argc, char *argv[])
{
	QTextCodec::setCodecForTr(QTextCodec::codecForName("windows-1251"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("windows-1251"));

#ifdef _WIN32
//#ifdef DEBUG
	WCHAR title[256];
	if(GetConsoleTitle(title,sizeof(title))==0)
	{
		AllocConsole();
		freopen("CONOUT$","wt", stdout);
		freopen("CONOUT$","wt", stderr);
		freopen("CONIN$","rt", stdin);
		GetConsoleTitle(title,sizeof(title));
		HWND hConsoleWindow = FindWindow(NULL,title);
		ShowWindow(hConsoleWindow,SW_SHOWMINNOACTIVE);
	}
//#endif
#endif
	QApplication a(argc, argv);
	PWLibProcess pwlibProcess;
	QtPhoneDlg *w = new QtPhoneDlg;
	w->show();
	int rc =  a.exec();
	delete w;
	return rc;
}
*/

class MyTest : public PProcess
{
  PCLASSINFO(MyTest, PProcess)

  public:
    MyTest();
    ~MyTest();

    virtual void Main();
};

PCREATE_PROCESS(MyTest);

MyTest::MyTest()
  : PProcess("MyTest", "MyTest", 1, 0, AlphaCode, 0)
{
}

MyTest::~MyTest()
{
}

void MyTest::Main()
{
	PArgList & args = GetArguments();
	int argCount = args.GetCount();
	const char **argV = (const char**)calloc(argCount+1, sizeof(const char*));
	bool fCreateConsole = false;
	for (int i = 0; i < argCount; i++)
	{
		argV[i] = (const char*)args.GetParameter(i);
		if(strcmp(argV[i], "-c")==0)
			fCreateConsole = true;
	}
	argV[argCount] = NULL;

	QTextCodec::setCodecForTr(QTextCodec::codecForName("windows-1251"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("windows-1251"));

#ifdef _WIN32
	if(fCreateConsole)
	{
		WCHAR title[256];
		if(GetConsoleTitle(title,sizeof(title))==0)
		{
			AllocConsole();
			freopen("CONOUT$","wt", stdout);
			freopen("CONOUT$","wt", stderr);
			freopen("CONIN$","rt", stdin);
			GetConsoleTitle(title,sizeof(title));
			HWND hConsoleWindow = FindWindow(NULL,title);
			ShowWindow(hConsoleWindow,SW_SHOWMINNOACTIVE);
		}
	}
#endif

	QApplication a(argCount, (char**)argV);

	QtPhoneDlg *w = new QtPhoneDlg;

	if (QSystemTrayIcon::isSystemTrayAvailable())
	{
 		QApplication::setQuitOnLastWindowClosed(false);
    }
	else
	{
		a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
		w->show();
	}

	int rc =  a.exec();
	delete w;
}
