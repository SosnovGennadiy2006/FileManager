#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), RESIZE_THRESHOLD(8), CLIENT_MARGIN(8)
{
    this->setAttribute(Qt::WA_NativeWindow);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint
                          | Qt::WindowTitleHint);
    this->setWindowTitle("MainWindow");

    this->cOpStatus = OperationType::NONE;
    this->forceCustomMove = this->forceCustomResize = false;

    this->setMinimumSize(QSize(500, 300));
    this->setMouseTracking(true);
    this->installEventFilter(this);

    this->privHW = new QHoverWatcher(this);

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(10);
    effect->setXOffset(0);
    effect->setYOffset(0);
    this->privHW->setGraphicsEffect(effect);

    this->privWnd = new QMainWindow();
    privWnd->setStyleSheet(QStringLiteral(
    "background: #fff;\n"
    ));
    this->privHWLayout = new QVBoxLayout(this);

    connect(this->privHW, &QHoverWatcher::entered, this, [this]{
        if (!(this->cOpStatus & OperationType::CUSTOM_RESIZE)){
            this->cOpStatus = OperationType::NONE;
            this->unsetCursor();
        }
    });

    this->privHWLayout->setContentsMargins(CLIENT_MARGIN, CLIENT_MARGIN, CLIENT_MARGIN, CLIENT_MARGIN);
    this->privHWLayout->setSpacing(0);
    this->privHWLayout->addWidget(this->privHW);

    this->privLayout = new QVBoxLayout(this->privHW);
    this->privLayout->setContentsMargins(0, 0, 0, 0);
    this->privLayout->setSpacing(0);
    this->privLayout->addWidget(this->privWnd);

    this->privHWLayout->addWidget(this->privHW);
    this->setTitleBar(new QCustomTitleBar(this));
    this->privHW->setLayout(this->privLayout);
    this->setLayout(this->privHWLayout);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setTitleBar(QCustomTitleBar *tb){
    tb->setParent(this);
    tb->setWindowIcon(QIcon("Images/icon.ico"));
    tb->setWindowTitle(this->windowTitle());

    connect(tb, &QCustomTitleBar::closeRequest, this, &MainWindow::close);
    connect(tb, &QCustomTitleBar::maximizeRequest, this, [this]{
        if (this->isMaximized())
        {
            this->privHWLayout->setContentsMargins(CLIENT_MARGIN, CLIENT_MARGIN, CLIENT_MARGIN, CLIENT_MARGIN);
            this->showNormal();
        }
        else
        {
            this->privHWLayout->setContentsMargins(0, 0, 0, 0);
            this->showMaximized();
        }
    });
    connect(tb, &QCustomTitleBar::minimizeRequest, this, &QWidget::showMinimized);
    connect(tb, &QCustomTitleBar::startWindowMoveRequest, this, [this](const QPoint &start){
        this->mPosCursor = start - this->geometry().topLeft();
    });
    connect(tb, &QCustomTitleBar::stopWindowMoveRequest, this, [this]{
       if (this->cOpStatus & OperationType::CUSTOM_MOVE) this->unsetCursor();
       this->cOpStatus = OperationType::NONE;
    });
    connect(tb, &QCustomTitleBar::changeWindowPositionRequest, this, [this](const QPoint &to){
        if (this->cOpStatus & OperationType::CUSTOM_MOVE) this->move(to - this->mPosCursor);
        else if (!this->isMoving()) {
            if (!this->forceCustomMove && this->windowHandle()->startSystemMove())
                this->cOpStatus = OperationType::SYSTEM_MOVE;
            else {
                this->cOpStatus = OperationType::CUSTOM_MOVE;
                this->setCursor(QCursor(Qt::SizeAllCursor));
            }
        }
    });

    connect(this, &QMainWindow::windowIconChanged, tb, &QWidget::setWindowIcon);
    connect(this, &QMainWindow::windowTitleChanged, tb, &QWidget::setWindowTitle);

    this->privLayout->insertWidget(0, tb);
    this->privTitleBar = tb;
}

bool MainWindow::eventFilter(QObject *, QEvent *event){
    switch (event->type()) {
    case QEvent::Leave:
    case QEvent::HoverLeave:
        if (!(this->cOpStatus & OperationType::CUSTOM_RESIZE)){
            this->cOpStatus = OperationType::NONE;
            this->unsetCursor();
        }
        break;
    case QEvent::MouseMove:
        if (this->cOpStatus & OperationType::CUSTOM_RESIZE)
            customMouseMoveEvent(static_cast<QMouseEvent*>(event));
        else if (!this->isMoving()) redefineCursor(EV_GLOBAL_POS(static_cast<QMouseEvent*>(event)));
        break;
    default: break;
    }
    return false;
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    this->redefineCursor(EV_GLOBAL_POS(event));
    if (event->button() & Qt::LeftButton && this->mLock){
        if (!this->forceCustomResize && this->windowHandle()->startSystemResize(this->mLock))
            this->cOpStatus = OperationType::SYSTEM_RESIZE;
        else {
            QPoint posCursor = EV_GLOBAL_POS(event);
            if (this->mLock & Qt::TopEdge) posCursor.ry() -= this->y();
            if (this->mLock & Qt::LeftEdge) posCursor.rx() -= this->x();
            if (this->mLock & Qt::RightEdge) posCursor.rx() -= (this->x() + this->width());
            if (this->mLock & Qt::BottomEdge) posCursor.ry() -= (this->y() + this->height());
            this->cOpStatus = OperationType::CUSTOM_RESIZE;
        }
    }
    QWidget::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event){
    this->cOpStatus = OperationType::NONE;
    this->redefineCursor(EV_GLOBAL_POS(event));
    QWidget::mouseReleaseEvent(event);
}

void MainWindow::customMouseMoveEvent(QMouseEvent *event){
    int gX = EV_GLOBAL_X(event), gY = EV_GLOBAL_Y(event);
    QPoint tL = this->geometry().topLeft(), bR = this->geometry().bottomRight();

    bool cRH = bR.x() - tL.x() > this->minimumWidth();
    bool cRV = bR.y() - tL.y() > this->minimumHeight();

    if (this->mLock & Qt::TopEdge && (cRV || gY < tL.y())) tL.ry() = gY;
    if (this->mLock & Qt::BottomEdge && (cRV || gY > bR.y())) bR.ry() = gY;
    if (this->mLock & Qt::LeftEdge && (cRH || gX < tL.x())) tL.rx() = gX;
    if (this->mLock & Qt::RightEdge && (cRH || gX > bR.x())) bR.rx() = gX;
    this->setGeometry(QRect(tL, bR));
}

void MainWindow::redefineCursor(const QPoint &pos){
    int x = pos.x() - this->x(), y = pos.y() - this->y();
    int bottom = this->height() - RESIZE_THRESHOLD, right = this->width() - RESIZE_THRESHOLD;

    Qt::Edges nFlags = {};
    if (x < RESIZE_THRESHOLD) nFlags |= Qt::LeftEdge;
    if (y < RESIZE_THRESHOLD) nFlags |= Qt::TopEdge;
    if (x > right) nFlags |= Qt::RightEdge;
    if (y > bottom) nFlags |= Qt::BottomEdge;

    switch (nFlags) {
    case Qt::LeftEdge:
    case Qt::RightEdge:
        this->setCursor(QCursor(Qt::SizeHorCursor)); break;
    case Qt::TopEdge:
    case Qt::BottomEdge:
        this->setCursor(QCursor(Qt::SizeVerCursor)); break;
    case Qt::TopEdge | Qt::LeftEdge:
    case Qt::BottomEdge | Qt::RightEdge:
        this->setCursor(QCursor(Qt::SizeFDiagCursor)); break;
    case Qt::TopEdge | Qt::RightEdge:
    case Qt::BottomEdge | Qt::LeftEdge:
        this->setCursor(QCursor(Qt::SizeBDiagCursor)); break;
    default: this->unsetCursor(); break;
    }

    this->mLock = nFlags;
}

void MainWindow::paintEvent(QPaintEvent *event){
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}
