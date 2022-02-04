#ifndef TITLE_BAR
#define TITLE_BAR

#include <QIcon>
#include <QSize>
#include <QFont>
#include <QEvent>
#include <QLabel>
#include <QPoint>
#include <QPointF>
#include <QPixmap>
#include <QWidget>
#include <QPainter>
#include <QtGlobal>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPushButton>
#include <QStyleOption>

#ifndef EV_GLOBAL_MACRO
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        #define EV_GLOBAL_X(event) event->globalX()
        #define EV_GLOBAL_Y(event) event->globalY()
        #define EV_GLOBAL_POS(event) event->globalPos()
    #else
        #define EV_GLOBAL_X(event) event->globalPosition().toPoint().x()
        #define EV_GLOBAL_Y(event) event->globalPosition().toPoint().y()
        #define EV_GLOBAL_POS(event) event->globalPosition().toPoint()
    #endif
    #define EV_GLOBAL_MACRO
#endif

namespace QCustomAttrs {
    enum WindowButton {
        Minimize = 0x01,
        Maximize = 0x02,
        Close    = 0x04
    };

    Q_DECLARE_FLAGS(WindowButtons, WindowButton)
    Q_DECLARE_OPERATORS_FOR_FLAGS(WindowButtons)
}

class QCustomTitleBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QCustomAttrs::WindowButtons windowButtons READ windowButtons WRITE setWindowButtons)
    Q_CLASSINFO("custom_obj_type", "QCustomTitleBar")
public:
    explicit QCustomTitleBar(QWidget *parent = nullptr);

    void setWindowButtons(QCustomAttrs::WindowButtons btns);
    inline QCustomAttrs::WindowButtons windowButtons() const { return this->mFrameButtons; }

    void setWindowButtonText(QCustomAttrs::WindowButton btn, const QString &text = "");
    void setWindowButtonEnabled(QCustomAttrs::WindowButton btn, bool enabled = true);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QPoint mPCursor;
    const QSize FRAME_BUTTON_SIZE;

    QCustomAttrs::WindowButtons mFrameButtons;

    QLabel lblWindowIcon;
    QLabel lblWindowTitle;
    QHBoxLayout mLayout;
    QPushButton btnMinimize;
    QPushButton btnMaximize;
    QPushButton btnClose;

signals:
    void closeRequest();
    void maximizeRequest();
    void minimizeRequest();

    void stopWindowMoveRequest();
    void startWindowMoveRequest(const QPoint &start);
    void changeWindowPositionRequest(const QPoint &to);
};

#endif // TITLE_BAR
