#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMenu>
#include <QRect>
#include <QTimer>
#include <QPoint>
#include <QStyle>
#include <QPointF>
#include <QObject>
#include <QWidget>
#include <QCursor>
#include <QWindow>
#include <QMenuBar>
#include <QPainter>
#include <QToolBar>
#include <QtGlobal>
#include <QStatusBar>
#include <QSizePolicy>
#include <QMetaMethod>
#include <QMetaObject>
#include <QDockWidget>
#include <QMainWindow>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QStyleOption>
#include <QDir>
#include <QGraphicsDropShadowEffect>

#include "widgets/titlebar.h"

class QHoverWatcher: public QWidget{
    Q_OBJECT

public:
    explicit QHoverWatcher(QWidget *parent = nullptr) : QWidget(parent) { this->installEventFilter(this); }

protected:
    bool eventFilter(QObject *, QEvent *event) override {
        switch (event->type()) {
            case QEvent::Enter:
            case QEvent::HoverEnter:
                emit entered();
                break;
        default: break;
        }
        return false;
    }

    void paintEvent(QPaintEvent *event) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
        QWidget::paintEvent(event);
    }

signals:
    void entered();
};

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    enum OperationType {
        NONE = 0x00,
        CUSTOM_MOVE = 0x01,
        CUSTOM_RESIZE = 0x02,
        SYSTEM_MOVE = 0x04,
        SYSTEM_RESIZE = 0x08,
    };

    Q_DECLARE_FLAGS(OperationTypes, OperationType)

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    inline bool isMoving(){ return this->cOpStatus & (OperationType::CUSTOM_MOVE | OperationType::SYSTEM_MOVE); }
    inline bool isResizing(){ return this->cOpStatus & (OperationType::CUSTOM_RESIZE | OperationType::SYSTEM_RESIZE); }

    inline QMainWindow* mainWindow(){ return this->privWnd; }
    inline QCustomTitleBar* titleBar(){ return this->privTitleBar; }

    void setTitleBar(QCustomTitleBar *tb);

    inline void setForceCustomMove(bool force = true){ this->forceCustomMove = force; }
    inline void setForceCustomResize(bool force = true){ this->forceCustomResize = force; }

protected:
    const int RESIZE_THRESHOLD, CLIENT_MARGIN;

    bool eventFilter(QObject *, QEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void customMouseMoveEvent(QMouseEvent* event);

private:
    bool forceCustomMove;
    bool forceCustomResize;

    OperationType cOpStatus;

    QVBoxLayout *privLayout;
    QVBoxLayout *privHWLayout;

    QMainWindow *privWnd;
    QHoverWatcher *privHW;
    QCustomTitleBar *privTitleBar;

    Qt::Edges mLock;
    QPoint mPosCursor;

    void redefineCursor(const QPoint &pos);
};

#endif // MAINWINDOW_H
