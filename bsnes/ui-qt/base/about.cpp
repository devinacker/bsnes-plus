#include "about.moc"
AboutWindow *aboutWindow;

AboutWindow::AboutWindow() {
  setObjectName("about-window");
  setWindowTitle("About");
  setGeometryString(&config().geometry.aboutWindow);
  application.windowList.append(this);

  #if defined(DEBUGGER)
  setStyleSheet("background: #c0c080");
  #elif defined(PROFILE_ACCURACY)
  setStyleSheet("background: #c08080");
  #elif defined(PROFILE_COMPATIBILITY)
  setStyleSheet("background: #8080c0");
  #elif defined(PROFILE_PERFORMANCE)
  setStyleSheet("background: #80c080");
  #endif

  layout = new QVBoxLayout;
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setMargin(UIStyle::WindowMargin);
  layout->setSpacing(UIStyle::WidgetSpacing);
  setLayout(layout);

  logo = new Logo;
  logo->setFixedSize(600, 106);
  layout->addWidget(logo);

  info = new QLabel(string() <<
    "<table width='100%'><tr>"
    "<td align='left'><b>bsnes-plus version</b> " << SNES::Info::Version << " (" __DATE__ ")</td>"
    "<td align='right'><b>" << SNES::Info::Profile <<" profile"
#if defined(DEBUGGER)
    " with debugger"
#endif
    "</b></td></tr><tr>"
    "<td align='left'><b>Project homepage:</b></td>"
    "<td align='right'><a href='http://bsnes.revenant1.net'>http://bsnes.revenant1.net</a></td>"
    "</tr><tr>"
    "<td align='left'><b>Based on bsnes-classic:</b></td>"
    "<td align='right'><a href='https://github.com/awjackson/bsnes-classic'>https://github.com/awjackson/bsnes-classic</a></td>"
    "</tr></table>"
  );
  layout->addWidget(info);
}

void AboutWindow::Logo::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  QPixmap pixmap(":/logo.png");
  painter.drawPixmap(0, 0, pixmap);
}
