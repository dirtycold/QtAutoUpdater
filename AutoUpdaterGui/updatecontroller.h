#ifndef UPDATECONTROLLER_H
#define UPDATECONTROLLER_H

#include <QObject>
#include <QAction>
#include <QStringList>
#include <QScopedPointer>
#include "qtautoupdater_global.h"

namespace QtAutoUpdater
{
	class Updater;
	//! @internal Private implementation
	class UpdateControllerPrivate;
	//! A class to show a controlled update GUI to the user
	class QTAUTOUPDATERSHARED_EXPORT UpdateController : public QObject
	{
		Q_OBJECT

		//! Holds the path of the attached maintenancetool
		Q_PROPERTY(QString maintenanceToolPath READ maintenanceToolPath CONSTANT FINAL)
		//! Holds the widget who's window should be used as parent for all dialogs
		Q_PROPERTY(QWidget* parentWindow READ parentWindow WRITE setParentWindow)
		//! Specifies whether the controller is currently active or not
		Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
		//! Specifies whether the controller should run the updater as admin or not
		Q_PROPERTY(bool runAsAdmin READ runAsAdmin WRITE setRunAsAdmin NOTIFY runAsAdminChanged)
		//! Holds the arguments to invoke the updater with
		Q_PROPERTY(QStringList updateRunArgs READ updateRunArgs WRITE setUpdateRunArgs RESET resetUpdateRunArgs)
		//! Specifies whether the update infos should be detailed or not
		Q_PROPERTY(bool detailedUpdateInfo READ isDetailedUpdateInfo WRITE setDetailedUpdateInfo)

	public:
		//! Defines the different display-levels of the dialog
		enum DisplayLevel {
			AutomaticLevel = 0,//!< The lowest level. Nothing will be displayed at all. The programm will be auto-closed.
			ExitLevel = 1,/*!< The whole updating works completly automatically without displaying anything. Only
						   *   a notification that updates are ready to install will be shown if updates are available.
						   */
			InfoLevel = 2,//!< Will show information about updates if available, nothing otherwise.
			ExtendedInfoLevel = 3,//!< Will show information about the update result, for both cases, updates and no updates.
			ProgressLevel = 4,//!< Shows a (modal) progress dialog while checking for updates.
			AskLevel = 5//!< The highest level. Will ask the user if he wants to check for updates before actually checking.
		};
		Q_ENUM(DisplayLevel)

		//! Constructs a new controller with a parent. Will be application modal
		explicit UpdateController(QObject *parent = nullptr);
		//! Constructs a new controller with a parent. Will modal to the parent window
		explicit UpdateController(QWidget *parentWindow, QObject *parent = nullptr);
		//! Constructs a new controller with an explicitly set path and a parent. Will modal to the parent window
		explicit UpdateController(const QString &maintenanceToolPath, QObject *parent = nullptr);
		//! Constructs a new controller with an explicitly set path and a parent. Will be application modal
		explicit UpdateController(const QString &maintenanceToolPath, QWidget *parentWindow, QObject *parent = nullptr);
		//! Destructor
		~UpdateController();

		//! Create a QAction to start this controller from
		QAction *createUpdateAction(QObject *parent);

		//! READ-Accessor for UpdateController::maintenanceToolPath
		QString maintenanceToolPath() const;
		//! READ-Accessor for UpdateController::parentWindow
		QWidget* parentWindow() const;
		//! WRITE-Accessor for UpdateController::parentWindow
		void setParentWindow(QWidget* parentWindow);
		//! READ-Accessor for UpdateController::currentDisplayLevel
		DisplayLevel currentDisplayLevel() const;
		//! READ-Accessor for UpdateController::running
		bool isRunning() const;
		//! READ-Accessor for UpdateController::runAsAdmin
		bool runAsAdmin() const;
		//! WRITE-Accessor for UpdateController::runAsAdmin
		void setRunAsAdmin(bool runAsAdmin, bool userEditable = true);
		//! READ-Accessor for UpdateController::updateRunArgs
		QStringList updateRunArgs() const;
		//! WRITE-Accessor for UpdateController::updateRunArgs
		void setUpdateRunArgs(QStringList updateRunArgs);
		//! RESET-Accessor for UpdateController::updateRunArgs
		void resetUpdateRunArgs();
		//! READ-Accessor for UpdateController::detailedUpdateInfo
		bool isDetailedUpdateInfo() const;
		//! WRITE-Accessor for UpdateController::detailedUpdateInfo
		void setDetailedUpdateInfo(bool detailedUpdateInfo);

		//! Returns the Updater object used by the controller
		const Updater *updater() const;

	public slots:
		//! Starts the controller with the specified level.
		bool start(DisplayLevel displayLevel = InfoLevel);
		//! Tries to cancel the controllers update
		bool cancelUpdate(int maxDelay = 3000);

		//! Schedules an update after a specific delay, optionally repeated
		int scheduleUpdate(int delaySeconds, bool repeated = false, DisplayLevel displayLevel = InfoLevel);
		//! Schedules an update for a specific timepoint
		int scheduleUpdate(const QDateTime &when, DisplayLevel displayLevel = InfoLevel);
		//! Cancels the update with taskId
		void cancelScheduledUpdate(int taskId);

	signals:
		//! NOTIFY-Accessor for UpdateController::running
		void runningChanged(bool running);
		//! NOTIFY-Accessor for UpdateController::runAsAdmin
		void runAsAdminChanged(bool runAsAdmin);

	private slots:
		void checkUpdatesDone(bool hasUpdates, bool hasError);
		void timerTriggered(const QVariant &parameter);

	private:
		QScopedPointer<UpdateControllerPrivate> d_ptr;
		Q_DECLARE_PRIVATE(UpdateController)
	};
}

#endif // UPDATECONTROLLER_H
