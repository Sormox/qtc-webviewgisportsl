#include <QApplication>
#include <QTabWidget>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QCloseEvent>
#include <QLabel>
#include <QTabBar>

class HtmlView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit HtmlView(QTabWidget *parent = nullptr)
        : QWebEngineView(parent), m_tab(parent)
    {
        connect(this, &QWebEngineView::loadStarted, this, &HtmlView::onLoadStarted);
        connect(this, &QWebEngineew::loadFinished, this, &HtmlView::onLoadFinished);
        connect(this->page(), &QWebEnginePage::loadFinished, this, &HtmlView::applyStyles);

    }
    void applyStyles(bool ok) {

        QString cssCode = R"(

        .js-orgname {
          visibility: hidden;
          position: relative;
        }

        .js-orgname::after {
            content: "Sign in to G.S.I - FIIA";
            visibility: visible;
            position: absolute;
            margin-left: 20px !important;
            top: 15px !important;
            left: 0;
            font-size: 18px;
        }

        #gnav-dist-esri-Australia-tm {
            display: none;
            position: relative;
        }

        .js-header::after {
            content: "";
            background-image: url('https://i.ibb.co/F4Qhg49/image.png');
            background-size: contain;
            background-repeat: no-repeat;
            position: absolute;
            top: 10px;
            right: 15px;
            width: 54px;
            height: 54px;
            visibility: visible;
        }

        @media screen and (max-width: 900px) {
            .js-header{
                padding-top: 5px !important;
                margin-right: 10px !important;
                margin-bottom: -10px !important;
            }

            .header-bar {
                padding: 11px 18px 11px 18px  !important;
                width: calc(1.5rem + 400px) !important;
                transform: translate(-1.88rem,-2.5rem) !important;
            }
        }
    )";

        if (ok) {
            this->page()->runJavaScript(QString("var styleElement = document.createElement('style');"
                                                "styleElement.innerHTML = `%1`;"
                                                "document.head.appendChild(styleElement);").arg(cssCode));
        }
    }

    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override
    {
        if (type == QWebEnginePage::WebBrowserTab)
        {
            HtmlView *newView = new HtmlView(m_tab);
            m_tab->addTab(newView, "Loading...");
            m_tab->setCurrentIndex(m_tab->count() - 1);
            return newView;
        }
        return QWebEngineView::createWindow(type);
    }

private slots:
    void onLoadStarted()
    {
        setTabTitle("Loading...");
    }

    void onLoadFinished(bool ok)
    {
        if (ok)
        {
            setTabTitle(this->title());
        }
        else
        {
            setTabTitle("Load Failed");
        }
    }

private:
    void setTabTitle(const QString &title)
    {
        int index = m_tab->indexOf(this);
        if (index != -1)
        {
            m_tab->setTabText(index, title);
        }
    }

private:
    QTabWidget *m_tab;
};

class Browser : public QWidget
{

    Q_OBJECT

public:
    explicit Browser(const QUrl &url, QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        QToolBar *toolbar = new QToolBar;
        toolbar->setIconSize(QSize(24, 24));
        QAction *homeAction = toolbar->addAction(QIcon(":/icons/home.png"), "Home");
        homeAction->setToolTip("Go to home page");
        QAction *backAction = toolbar->addAction(QIcon(":/icons/back.png"), "Back");

        QPixmap pixmap(":/icons/logo.png");
        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(pixmap);
        iconLabel->setScaledContents(true);
        iconLabel->setFixedSize(42, 42);
        toolbar->addWidget(iconLabel);

        QAction *forwardAction = toolbar->addAction(QIcon(":/icons/forward.png"), "Forward");
        forwardAction->setToolTip("Go to the next page");
        QAction *refreshAction = toolbar->addAction(QIcon(":/icons/refresh.png"), "Refresh");
        refreshAction->setToolTip("Reload the current page");

        QWidget *spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        toolbar->insertWidget(homeAction, spacer);

        QWidget *spacer2 = new QWidget();
        spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        toolbar->addWidget(spacer2);
        layout->addWidget(toolbar);

        m_tabWidget = new QTabWidget;
        m_tabWidget->setStyleSheet("QTabBar::close-button { image: url(:/icons/close.png); width: 16px; height: 16px; }");
        m_tabWidget->setTabsClosable(true);
        HtmlView *view = new HtmlView(m_tabWidget);
        view->load(url);
        m_tabWidget->addTab(view, "Loading...");
        layout->addWidget(m_tabWidget);

        connect(homeAction, &QAction::triggered, this, [&]
                {
            HtmlView *currentView = dynamic_cast<HtmlView*>(m_tabWidget->currentWidget());
            if (currentView) {
                currentView->load(url);
                m_tabWidget->setTabText(m_tabWidget->currentIndex(), "Loading...");
            } });
        connect(backAction, &QAction::triggered, [=]()
                {
            HtmlView *view = dynamic_cast<HtmlView*>(m_tabWidget->currentWidget());
            if (view) view->back(); });
        connect(forwardAction, &QAction::triggered, [=]()
                {
            HtmlView *view = dynamic_cast<HtmlView*>(m_tabWidget->currentWidget());
            if (view) view->forward(); });
        connect(refreshAction, &QAction::triggered, [=]()
                {
            HtmlView *view = dynamic_cast<HtmlView*>(m_tabWidget->currentWidget());
            if (view) view->reload(); });
        connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
    }

public slots:
    void closeTab(int index)
    {
        if (m_tabWidget->count() > 1)
        {
            m_tabWidget->removeTab(index);
        }
    }
    void setTabTitle(const QString &title)
    {
        int index = m_tabWidget->currentIndex();
        if (index != -1)
        {
            m_tabWidget->setTabText(index, title);
        }
    }

private:
    QTabWidget *m_tabWidget;
};

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/logo.png"));

    // QUrl url("https://gsi.fiia.gov.iq/portal");
    QUrl url("https://www.google.com");

    Browser browser(url);
    browser.show();

    browser.resize(1366, 728);

    browser.show();

    return a.exec();
}

#include "main.moc"
