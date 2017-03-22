/*
  Greesound
  Copyright (C) 2015 Grame

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Centre national de création musicale
  11, cours de Verdun Gensoul 69002 Lyon - France

*/

#include <QQuickItem>
#ifdef ANDROID
# include <QtAndroid>
# include <QAndroidJniObject>
#endif

#include "SensorAppl.h"

extern const char* kGreensoundsAddr;
extern const char* kButtonsAddr;
const float kVersion = 1.13f;
const char* kVersionStr = "1.13";

using namespace std;

//------------------------------------------------------------------------
OSCListener::OSCListener(SensorAppl* appl, int port)
	: fAppl(appl), fRunning(false)
{
	try {
		fSocket = new UdpListeningReceiveSocket(IpEndpointName( IpEndpointName::ANY_ADDRESS, port ), this);
	}
	catch (std::runtime_error e) {
		cerr << "can't create upd socket: " << e.what() << endl;
		fSocket = 0;
	}
}

OSCListener::~OSCListener()	{
	if (fSocket) fSocket->AsynchronousBreak();
	delete fSocket;
}

//------------------------------------------------------------------------
void OSCListener::run()
{
	fRunning = true;
	try {
		if (fSocket) fSocket->Run();
	}
	catch (osc::Exception e) {
		cerr << "osc error: " << e.what() << endl;
	}
	fRunning = false;
}

//------------------------------------------------------------------------
SensorAppl::~SensorAppl()
{
	fSensors.send(kGreensoundsAddr, fSensors.ipstr(), "bye");
	fListener.terminate();
}

#ifdef ANDROID

static void keepScreenOn(bool on)
{
	QtAndroid::runOnAndroidThread([on]{
		QAndroidJniObject activity = QtAndroid::androidActivity();
		if (activity.isValid()) {
		  QAndroidJniObject window =
			  activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

		  if (window.isValid()) {
			const int FLAG_KEEP_SCREEN_ON = 128;
			if (on) {
			  window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
			} else {
			  window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
			}
		  }
		}
//		QAndroidJniEnvironment env;
//		if (env->ExceptionCheck()) {
//		  env->ExceptionClear();
//		}
	});
}
#else
static void keepScreenOn(bool )	{}
#endif


//#define TESTMOTOE
//------------------------------------------------------------------------
void SensorAppl::start()
{
	keepScreenOn(true);
	fView.setSource(QUrl("qrc:/GSinit.qml"));
	fView.rootContext()->setContextProperty("sensors", &fSensors);
	fView.show();

#ifndef MACOS
	bool ret = fSensors.initSensor();
	if (!ret) {
		fView.setSource(QUrl("qrc:/failsensor.qml"));
	}
    else {
//		fView.setSource(QUrl("qrc:/GSinit.qml"));
//		fView.rootContext()->setContextProperty("sensors", &fSensors);
//		fView.show();
		fTimerID = startTimer(1000);
		fListener.start();
	}
#else
	fTimerID = startTimer(1000);
	fListener.start();
#endif
	connect((QObject*)fView.engine(), SIGNAL(quit()), this, SLOT(quit()));
	connect((QObject*)this, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this, SLOT(stateChanged(Qt::ApplicationState)));
}

//------------------------------------------------------------------------
void SensorAppl::stateChanged(Qt::ApplicationState state)
{
#ifndef MACOS
	if ((state == Qt::ApplicationSuspended) || ((state == Qt::ApplicationInactive))) {
		quit();
	}
#endif
}

//------------------------------------------------------------------------
void SensorAppl::greensound()
{
	fView.setSource(QUrl("qrc:/GSwait.qml"));
	fView.rootContext()->setContextProperty("sensors", &fSensors);
	fSensors.start((QObject*)fView.rootObject());
	fRunning = true;
}

//------------------------------------------------------------------------
void SensorAppl::wait()
{
	fWait = true;
	fUISwitch = true;
	fSensors.stop();
}

//------------------------------------------------------------------------
void SensorAppl::play()
{
	fWait = false;
	fUISwitch = true;
	fSensors.start();
	fSensors.pmode(false);
}

//------------------------------------------------------------------------
void SensorAppl::setButtons(int b1, int b2, int b3)
{
	fButtonsState[0] = b1;
	fButtonsState[1] = b2;
	fButtonsState[2] = b3;
	fSetButtons = true;
}

//------------------------------------------------------------------------
void SensorAppl::setButtonState (QObject * button, bool state)
{
	if (button) {
		button->setProperty ("opacity", state ? 1 : 0.4);
		button->setProperty ("enabled", state);
	}
}

//------------------------------------------------------------------------
void SensorAppl::timerEvent(QTimerEvent*)
{
	static int ntry = 1;
	if (fRunning) {
		if (fUISwitch) {
			if (fWait)
				fView.setSource(QUrl("qrc:/GSwait.qml"));
			else
				fView.setSource(QUrl("qrc:/greensounds.qml"));
			fUISwitch = false;
		}
		fSensors.send(kGreensoundsAddr, fSensors.ipstr(), fWait ? "wait" : "play");
		if (fSetButtons) {
			QQuickItem* root = fView.rootObject();
			setButtonState (root->findChild<QObject*>("b1"), fButtonsState[0]);
			setButtonState (root->findChild<QObject*>("b2"), fButtonsState[1]);
			setButtonState (root->findChild<QObject*>("b3"), fButtonsState[2]);
			fSetButtons = false;
		}
	}
	else if (fSensors.connected() ) {
		if (fSensors.network()) {
			greensound();
		}
		else
			fView.setSource(QUrl("qrc:/failnetwork.qml"));
	}
	else if (ntry < 5) {
		fSensors.hello();
		ntry++;
	}
	else if (fSensors.skip()) {
		greensound();
	}
	else fView.setSource(QUrl("qrc:/failnetwork.qml"));
}

//------------------------------------------------------------------------
void OSCListener::ProcessMessage( const osc::ReceivedMessage& m, const IpEndpointName& src )
{
	string address(m.AddressPattern());
	
	try {
    if (address == kGreensoundsAddr) {
        osc::ReceivedMessageArgumentIterator i = m.ArgumentsBegin();
		while (i != m.ArgumentsEnd()) {
			if (i->IsString()) {
				string msg(i->AsStringUnchecked());
				if (msg == "hello") {
					char buff[120];
					src.AddressAsString(buff);
					fAppl->connect_to(buff);
				}
				else if (msg == "version")
					fAppl->sensors()->send (kGreensoundsAddr, "version", kVersion);
				else if (msg == "wait")
					fAppl->wait();
				else if (msg == "play")
					fAppl->play();
				else if (msg == "quit")
					fAppl->quit();
				
			}
			else if (i->IsInt32()) {
			}
			else if (i->IsFloat()) {
			}
			i++;
		}
	}
	else if ((address == kButtonsAddr) && (m.ArgumentCount() == 3)) {
		osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
		osc::int32 b1, b2, b3;
		args >> b1 >> b2 >> b3;
		fAppl->setButtons( b1, b2, b3);
	}
	}
	catch(std::exception e) {}
}
