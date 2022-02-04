#include "titlebar.h"

QCustomTitleBar::QCustomTitleBar(QWidget *parent) :
    QWidget(parent), FRAME_BUTTON_SIZE(30, 30)
{
    this->mFrameButtons = QCustomAttrs::Close | QCustomAttrs::Maximize | QCustomAttrs::Minimize;

    this->setStyleSheet(QStringLiteral(
        "QPushButton {\n"
        "   min-height: 20px;\n"
        "   background: #1E90FF;\n"
        "   border: none;\n"
        "   color: white;\n"
        "}\n"
        "QPushButton::hover {\n"
        "   background: #1A7EDF;\n"
        "}\n"
        "QPushButton::pressed, QPushButton::!enabled {\n"
        "   background: #166CBF;\n"
        "   color: white;\n"
        "}\n"
        "QLabel#lblWindowTitle {\n"
        "   margin-left: 8px;\n"
        "   color: #ffffff;\n"
        "}\n"
        "QCustomTitleBar { background: #1E90FF; }\n"
    ));

    this->lblWindowTitle.setText("QCustomWindow");
    this->lblWindowTitle.setAlignment(Qt::AlignCenter);

    QFont font(QString::fromUtf8("Marlett"));

    this->btnClose.setFont(font);
    this->btnMaximize.setFont(font);
    this->btnMinimize.setFont(font);

    this->btnClose.setText("r");
    this->btnMaximize.setText("1");
    this->btnMinimize.setText("0");
    this->btnClose.setStyleSheet(QStringLiteral(
        "QPushButton::hover {\n"
        "   background: #DF1A2E;\n"
        "}\n"
        "QPushButton::pressed, QPushButton::!enabled {\n"
        "   background: #DF1A2E;\n"
        "}\n"
    ));

    this->lblWindowIcon.setMaximumSize(QSize(FRAME_BUTTON_SIZE.width() - 8, FRAME_BUTTON_SIZE.height() - 8));
    this->lblWindowIcon.setMinimumSize(QSize(FRAME_BUTTON_SIZE.width() - 8, FRAME_BUTTON_SIZE.height() - 8));
    this->lblWindowIcon.setPixmap(QPixmap("../QtWidgetsApp/Images/icon.png"));

    this->btnClose.setObjectName("btnClose");
    this->btnMaximize.setObjectName("btnMaximize");
    this->btnMinimize.setObjectName("btnMinimize");
    this->lblWindowIcon.setObjectName("lblWindowIcon");
    this->lblWindowTitle.setObjectName("lblWindowTitle");

    this->btnClose.setMaximumSize(FRAME_BUTTON_SIZE);
    this->btnClose.setMinimumSize(FRAME_BUTTON_SIZE);
    this->btnMaximize.setMaximumSize(FRAME_BUTTON_SIZE);
    this->btnMaximize.setMinimumSize(FRAME_BUTTON_SIZE);
    this->btnMinimize.setMaximumSize(FRAME_BUTTON_SIZE);
    this->btnMinimize.setMinimumSize(FRAME_BUTTON_SIZE);

    this->mLayout.addWidget(&this->lblWindowIcon);
    this->mLayout.addWidget(&this->lblWindowTitle, 1);
    this->mLayout.addWidget(&this->btnMinimize);
    this->mLayout.addWidget(&this->btnMaximize);
    this->mLayout.addWidget(&this->btnClose);
    this->mLayout.setContentsMargins(4, 0, 0, 0);
    this->mLayout.setSpacing(0);

    this->setLayout(&this->mLayout);

    connect(&this->btnClose, &QPushButton::clicked, this, [this]{ emit this->closeRequest(); });
    connect(&this->btnMinimize, &QPushButton::clicked, this, [this]{ emit this->minimizeRequest(); });
    connect(&this->btnMaximize, &QPushButton::clicked, this, [this]{
        if (this->btnMaximize.text() == "1")
            this->btnMaximize.setText("2");
        else
            this->btnMaximize.setText("1");
        emit this->maximizeRequest();
    });

    connect(this, &QWidget::windowTitleChanged, &this->lblWindowTitle, &QLabel::setText);
    connect(this, &QWidget::windowIconChanged, this, [this](const QIcon &icon){
        this->lblWindowIcon.setPixmap(icon.pixmap(FRAME_BUTTON_SIZE));
        this->lblWindowIcon.setVisible(!icon.isNull());
    });

    this->setMaximumHeight(FRAME_BUTTON_SIZE.height());
    this->setMinimumHeight(FRAME_BUTTON_SIZE.height());
}

void QCustomTitleBar::setWindowButtons(QCustomAttrs::WindowButtons btns){
    this->mFrameButtons = btns;
    this->btnClose.setVisible(btns & QCustomAttrs::Close);
    this->btnMaximize.setVisible(btns & QCustomAttrs::Maximize);
    this->btnMinimize.setVisible(btns & QCustomAttrs::Minimize);
}

void QCustomTitleBar::setWindowButtonText(QCustomAttrs::WindowButton btn, const QString &text){
    switch (btn) {
    case QCustomAttrs::Close: this->btnClose.setText(text); break;
    case QCustomAttrs::Maximize: this->btnMaximize.setText(text); break;
    case QCustomAttrs::Minimize: this->btnMinimize.setText(text); break;
    }
}

void QCustomTitleBar::setWindowButtonEnabled(QCustomAttrs::WindowButton btn, bool enabled){
    switch (btn) {
    case QCustomAttrs::Close: this->btnClose.setEnabled(enabled); break;
    case QCustomAttrs::Maximize: this->btnMaximize.setEnabled(enabled); break;
    case QCustomAttrs::Minimize: this->btnMinimize.setEnabled(enabled); break;
    }
}

void QCustomTitleBar::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton) emit changeWindowPositionRequest(EV_GLOBAL_POS(event));
    QWidget::mouseMoveEvent(event);
}

void QCustomTitleBar::mousePressEvent(QMouseEvent *event){
    if (event->button() & Qt::LeftButton) emit startWindowMoveRequest(EV_GLOBAL_POS(event));
    QWidget::mousePressEvent(event);
}

void QCustomTitleBar::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() & Qt::LeftButton) emit stopWindowMoveRequest();
    QWidget::mouseReleaseEvent(event);
}

void QCustomTitleBar::mouseDoubleClickEvent(QMouseEvent *event){
    if (mFrameButtons & QCustomAttrs::Maximize && btnMaximize.isEnabled()
            && event->buttons() & Qt::LeftButton) {
        emit maximizeRequest();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void QCustomTitleBar::paintEvent(QPaintEvent *event){
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}
