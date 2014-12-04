#ifndef APP_H
#define APP_H

#include "OGRE/Ogre.h"
#include "OIS/OIS.h"
#include "ARLib/Oculus/Rift.h"
#include "Scene.h"

class App : public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener
{
	public:
		App(bool showDebugWindow);
		~App();

		void quit();

		bool keyPressed(const OIS::KeyEvent& e);
		bool keyReleased(const OIS::KeyEvent& e);
		bool mouseMoved(const OIS::MouseEvent& e);
		bool mousePressed(const OIS::MouseEvent& e, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent& e, OIS::MouseButtonID id);

		bool frameRenderingQueued(const Ogre::FrameEvent& evt);

		bool update();

	private:
		void initOgre(bool showDebugWindow);
		void quitOgre();
		void initOIS();
		void quitOIS();
		void initRift();
		void quitRift();
		void createViewports();

		OIS::Keyboard* mKeyboard;
		OIS::Mouse* mMouse;

		Ogre::Root* mRoot;

		Ogre::RenderWindow* mWindow;
		Ogre::RenderWindow* mSmallWindow;
		Ogre::Viewport* mViewportL;
		Ogre::Viewport* mViewportR;

		bool mShutdown;

		Scene* mScene;

		bool mRiftAvailable;
		ARLib::Rift* mRift;
};

#endif
