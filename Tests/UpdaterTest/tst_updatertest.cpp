#include <updater.h>
#include <QVector>
#include <functional>
using namespace QtAutoUpdater;

inline bool operator==(const QtAutoUpdater::Updater::UpdateInfo &a, const QtAutoUpdater::Updater::UpdateInfo &b) {
	return (a.name == b.name &&
			a.version == b.version &&
			a.size == b.size);
}

#include <QtTest>
#include <QCoreApplication>
#include <QSignalSpy>

#define TEST_DELAY 1000

class UpdaterTest : public QObject
{
	Q_OBJECT

public:
	inline UpdaterTest() {}

private Q_SLOTS:
	void testUpdaterInitState();

	void testUpdateCheck_data();
	void testUpdateCheck();

private:
	Updater *updater;
	QSignalSpy *checkSpy;
	QSignalSpy *runningSpy;
	QSignalSpy *updateInfoSpy;
};

void UpdaterTest::testUpdaterInitState()
{
	this->updater = new Updater(this);

	//error state
	QVERIFY(this->updater->exitedNormally());
	QCOMPARE(this->updater->errorCode(), EXIT_SUCCESS);
	QVERIFY(this->updater->errorLog().isEmpty());

	//properties
#if defined(Q_OS_WIN32)
	QCOMPARE(this->updater->maintenanceToolPath(), QStringLiteral("./maintenancetool.exe"));
#elif defined(Q_OS_OSX)
	QCOMPARE(this->updater->maintenanceToolPath(), QStringLiteral("../../maintenancetool.app/Contents/MacOS/maintenancetool"));
#elif defined(Q_OS_UNIX)
    QCOMPARE(this->updater->maintenanceToolPath(), QStringLiteral("./maintenancetool"));
#endif
	QCOMPARE(this->updater->isRunning(), false);
	QVERIFY(this->updater->updateInfo().isEmpty());

	delete this->updater;
}

void UpdaterTest::testUpdateCheck_data()
{
	QTest::addColumn<QString>("toolPath");
	QTest::addColumn<bool>("hasUpdates");
	QTest::addColumn<QList<Updater::UpdateInfo>>("updates");

#ifdef Q_OS_WIN
	QList<Updater::UpdateInfo> updates;
	updates += {"IcoDroid", QVersionNumber::fromString("1.1.2"), 52459213ull};
	QTest::newRow("D:/Program Files/IcoDroid") << "D:/Program Files/IcoDroid/maintenancetool"
											   << true
											   << updates;

	updates.clear();
	QTest::newRow("C:/Qt") << "C:/Qt/MaintenanceTool"
						   << false
						   << updates;
#elif defined(Q_OS_OSX)
	QList<Updater::UpdateInfo> updates;
	updates += {"IcoDroid", QVersionNumber::fromString("1.1.2"), 23391377ull};
	QTest::newRow("/Applications/IcoDroid.app") << "/Applications/IcoDroid.app/maintenancetool"
												<< true
												<< updates;

	updates.clear();
	QTest::newRow("/Users/sky/Qt") << "/Users/sky/Qt/MaintenanceTool"
								   << false
								   << updates;
#elif defined(Q_OS_UNIX)
	QList<Updater::UpdateInfo> updates;
	updates += {"IcoDroid", QVersionNumber::fromString("1.1.2"), 55979275ull};
	QTest::newRow("/home/sky/IcoDroid") << "/home/sky/IcoDroid/maintenancetool"
										<< true
										<< updates;

	updates.clear();
	QTest::newRow("/home/sky/Qt") << "/home/sky/Qt/MaintenanceTool"
								  << false
								  << updates;
#endif
}

void UpdaterTest::testUpdateCheck()
{
	QFETCH(QString, toolPath);
	QFETCH(bool, hasUpdates);
	QFETCH(QList<Updater::UpdateInfo>, updates);

	this->updater = new Updater(toolPath, this);
	QVERIFY(this->updater);
	this->checkSpy = new QSignalSpy(this->updater, &Updater::checkUpdatesDone);
	QVERIFY(this->checkSpy->isValid());
	this->runningSpy = new QSignalSpy(this->updater, &Updater::runningChanged);
	QVERIFY(this->runningSpy->isValid());
	this->updateInfoSpy = new QSignalSpy(this->updater, &Updater::updateInfoChanged);
	QVERIFY(this->updateInfoSpy->isValid());

	//start the check updates
	QVERIFY(!this->updater->isRunning());
	QVERIFY(this->updater->checkForUpdates());

	//runnig should have changed to true
	QCOMPARE(this->runningSpy->size(), 1);
	QVERIFY(this->runningSpy->takeFirst()[0].toBool());
	QVERIFY(this->updater->isRunning());
	QVERIFY(this->updateInfoSpy->takeFirst()[0].value<QList<Updater::UpdateInfo>>().isEmpty());

	//wait max 2 min for the process to finish
	QVERIFY(this->checkSpy->wait(120000));

	//show error log before continuing checking
	QByteArray log = this->updater->errorLog();
	if(!log.isEmpty())
		qWarning() << "Error log:" << log;

	//check if the finished signal is without error
	QCOMPARE(this->checkSpy->size(), 1);
	QVariantList varList = this->checkSpy->takeFirst();
	QVERIFY(this->updater->exitedNormally());
	QCOMPARE(this->updater->errorCode(), hasUpdates ? EXIT_SUCCESS : EXIT_FAILURE);
	QCOMPARE(varList[1].toBool(), !hasUpdates);

	//verifiy the "hasUpdates" and "updates" are as expected
	QCOMPARE(varList[0].toBool(), hasUpdates);
	QCOMPARE(this->updater->updateInfo(), updates);
	if(hasUpdates) {
		QCOMPARE(this->updateInfoSpy->size(), 1);
		QCOMPARE(this->updateInfoSpy->takeFirst()[0].value<QList<Updater::UpdateInfo>>(), updates);
	}

	//runnig should have changed to false
	QCOMPARE(this->runningSpy->size(), 1);
	QVERIFY(!this->runningSpy->takeFirst()[0].toBool());
	QVERIFY(!this->updater->isRunning());

	//verifiy all signalspies are empty
	QVERIFY(this->checkSpy->isEmpty());
	QVERIFY(this->runningSpy->isEmpty());
	QVERIFY(this->updateInfoSpy->isEmpty());

	//-----------schedule mechanism---------------

	int kId = this->updater->scheduleUpdate(1, true);//every 1 minute
	QVERIFY(kId);
	this->updater->cancelScheduledUpdate(kId);

	kId = this->updater->scheduleUpdate(QDateTime::currentDateTime().addSecs(5));
	QVERIFY(this->updater->scheduleUpdate(QDateTime::currentDateTime().addSecs(2)));
	this->updater->cancelScheduledUpdate(kId);

	//wait for the update to start
	QVERIFY(this->runningSpy->wait(2000 + TEST_DELAY));
	//should be running
	QVERIFY(this->runningSpy->size() > 0);
	QVERIFY(this->runningSpy->takeFirst()[0].toBool());
	//wait for it to finish if not running
	if(this->runningSpy->isEmpty())
		QVERIFY(this->runningSpy->wait(120000));
	//should have stopped
	QCOMPARE(this->runningSpy->size(), 1);
	QVERIFY(!this->runningSpy->takeFirst()[0].toBool());

	//wait for the canceled one (max 5 secs)
	QVERIFY(!this->runningSpy->wait(5000 + TEST_DELAY));

	//verifiy the runningSpy is empty
	QVERIFY(this->runningSpy->isEmpty());
	//clear the rest
	this->checkSpy->clear();
	this->updateInfoSpy->clear();

	delete this->updateInfoSpy;
	delete this->runningSpy;
	delete this->checkSpy;
	delete this->updater;
}

QTEST_GUILESS_MAIN(UpdaterTest)

#include "tst_updatertest.moc"
