#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "sqt/MidClickTabBar.h"
#include "sqt/CustomHeadTabWidget.h"
#include <mous/MediaItem.h>
#include "SimplePlayListView.h"
using namespace std;
using namespace sqt;
using namespace mous;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mTimerUpdateUi(new QTimer),
    mUpdateInterval(500),
    mMediaItem(NULL),
    mSliderPlayingPreempted(false)
{
    ui->setupUi(this);    
    initMousCore();
    initMyUi();
    initQtSlots();
}

MainWindow::~MainWindow()
{
    if (mPlayer.GetStatus() == PlayerStatus::Playing) {
        mPlayer.Stop();
    }
    if (mTimerUpdateUi != NULL) {
        if (mTimerUpdateUi->isActive())
            mTimerUpdateUi->stop();
        delete mTimerUpdateUi;
    }

    delete ui;
}

void MainWindow::initMousCore()
{
    mPluginMgr.LoadPluginDir("./plugins");
    vector<string> pathList;
    mPluginMgr.GetPluginPath(pathList);

    for (size_t i = 0; i < pathList.size(); ++i) {
        qDebug() << ">> " << pathList[i].c_str();
    }

    vector<PluginAgent*> packAgentList;
    mPluginMgr.GetPluginAgents(packAgentList, PluginType::MediaPack);
    qDebug() << ">> MediaPack count:" << packAgentList.size();

    vector<PluginAgent*> tagAgentList;
    mPluginMgr.GetPluginAgents(tagAgentList, PluginType::TagParser);
    qDebug() << ">> TagParser count:" << tagAgentList.size();

    for (size_t i = 0; i < packAgentList.size(); ++i) {
        mLoader.RegisterPluginAgent(packAgentList[i]);
    }

    for (size_t i = 0; i < tagAgentList.size(); ++i) {
        mLoader.RegisterPluginAgent(tagAgentList[i]);
    }

    vector<PluginAgent*> decoderAgentList;
    mPluginMgr.GetPluginAgents(decoderAgentList, PluginType::Decoder);
    qDebug() << ">> Decoder count:" << decoderAgentList.size();

    vector<PluginAgent*> rendererAgentList;
    mPluginMgr.GetPluginAgents(rendererAgentList, PluginType::Renderer);
    qDebug() << ">> Renderer count:" << rendererAgentList.size();

    mPlayer.SetRendererDevice("/dev/dsp");

    mPlayer.RegisterPluginAgent(rendererAgentList[0]);
    for (size_t i = 0; i < decoderAgentList.size(); ++i) {
        mPlayer.RegisterPluginAgent(decoderAgentList[i]);
    }

    mPlayer.SigFinished.Connect(&MainWindow::slotPlayerStopped, this);

    deque<MediaItem*> mediaList;
    mLoader.LoadMedia("/home/shen/project/mous/build/test.mp3", mediaList);

    mMediaItem = mediaList[0];
    mPlayer.Open(mMediaItem->url);
    qDebug() << mMediaItem->url.c_str();
}

void MainWindow::initMyUi()
{
    // Playing & Paused icon
    mIconPlaying.addFile(QString::fromUtf8(":/img/resource/play.png"), QSize(), QIcon::Normal, QIcon::On);
    mIconPaused.addFile(QString::fromUtf8(":/img/resource/pause.png"), QSize(), QIcon::Normal, QIcon::On);

    // Play mode button

    // PlayList View
    mBarPlayList = new MidClickTabBar();
    mWidgetPlayList = new CustomHeadTabWidget();
    mWidgetPlayList->setTabBar(mBarPlayList);
    mWidgetPlayList->setMovable(true);
    ui->layoutPlayList->addWidget(mWidgetPlayList);

    mWidgetPlayList->addTab(new QLabel("test"), "default");

    // Status bar buttons
    mBtnPreference = new QToolButton(ui->barStatus);
    mBtnPreference->setAutoRaise(true);
    mBtnPreference->setText("P");
    mBtnPreference->setToolTip(tr("Preference"));

    ui->barStatus->addPermanentWidget(mBtnPreference, 0);
}

void MainWindow::initQtSlots()
{
    connect(mTimerUpdateUi, SIGNAL(timeout()), this, SLOT(slotUpdateUi()));

    connect(ui->btnPlay, SIGNAL(clicked()), this, SLOT(slotBtnPlay()));
    connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(slotBtnStop()));

    connect(ui->sliderPlaying, SIGNAL(sliderPressed()), this, SLOT(slotSliderPlayingPressed()));
    connect(ui->sliderPlaying, SIGNAL(sliderReleased()), this, SLOT(slotSliderPlayingReleased()));
    connect(ui->sliderPlaying, SIGNAL(valueChanged(int)), this, SLOT(slotSliderPlayingValueChanged(int)));

    connect(mBarPlayList, SIGNAL(sigMidClick(int)), this, SLOT(slotBarPlayListMidClick(int)));
    connect(mWidgetPlayList, SIGNAL(sigDoubleClick()), this, SLOT(slotWidgetPlayListDoubleClick()));
}

void MainWindow::formatTime(QString& str, int ms)
{
    int sec = ms/1000;
    str.sprintf("%.2d:%.2d", (int)(sec/60), (int)(sec%60));
}

/* MousCore slots */
void MainWindow::slotPlayerStopped()
{
    qDebug() << "Stopped!";
}

/* Qt slots */
void MainWindow::slotUpdateUi()
{
    // Update statusbar.
    int total = mPlayer.GetRangeDuration();
    int ms = mPlayer.GetOffsetMs();
    int kbps = mPlayer.GetBitRate();

    mStatusMsg.sprintf("%d kbps | %.2d:%.2d/%.2d:%.2d",
                 kbps,
                 ms/1000/60, ms/1000%60, total/1000/60, total/1000%60);

    ui->barStatus->showMessage(mStatusMsg);

    // Update slider.
    if (!mSliderPlayingPreempted) {
        int percent = (double)ms / total * ui->sliderPlaying->maximum();
        ui->sliderPlaying->setSliderPosition(percent);
    }
}

void MainWindow::slotBtnPlay()
{
    qDebug() << mPlayer.GetStatus();

    switch (mPlayer.GetStatus()) {
    case PlayerStatus::Closed:
        if (mMediaItem != NULL) {
            mPlayer.Open(mMediaItem->url);
            slotBtnPlay();
        }
        break;

    case PlayerStatus::Playing:
        mPlayer.Pause();
        mTimerUpdateUi->stop();
        ui->btnPlay->setIcon(mIconPlaying);
        break;

    case PlayerStatus::Paused:
        mTimerUpdateUi->start(mUpdateInterval);
        mPlayer.Resume();
        ui->btnPlay->setIcon(mIconPaused);
        break;

    case PlayerStatus::Stopped:
        mTimerUpdateUi->start(mUpdateInterval);
        if (mMediaItem->hasRange)
            mPlayer.Play(mMediaItem->msBeg, mMediaItem->msEnd);
        else
            mPlayer.Play();
        ui->btnPlay->setIcon(mIconPaused);
        break;
    }
}

void MainWindow::slotBtnStop()
{
    qDebug() << mPlayer.GetStatus();

    mPlayer.Stop();
    mTimerUpdateUi->stop();
}

void MainWindow::slotSliderPlayingPressed()
{
    mSliderPlayingPreempted = true;
}

void MainWindow::slotSliderPlayingReleased()
{
    mSliderPlayingPreempted = false;
}

void MainWindow::slotSliderPlayingValueChanged(int val)
{
    if (!mSliderPlayingPreempted)
        return;

    uint64_t ms = (double)val / ui->sliderPlaying->maximum() * mPlayer.GetRangeDuration();
    mPlayer.Seek(mPlayer.GetRangeBegin() + ms);
}

void MainWindow::slotBarPlayListMidClick(int index)
{
    QWidget* w = mWidgetPlayList->widget(index);
    mWidgetPlayList->removeTab(index);
    delete w;
    mBarPlayList->setFocus();
}

void MainWindow::slotWidgetPlayListDoubleClick()
{
    SimplePlayListView* w = new SimplePlayListView(this);

    mWidgetPlayList->addTab(w, QString::number(mWidgetPlayList->count()));
    mWidgetPlayList->setCurrentIndex(mWidgetPlayList->count()-1);
}
