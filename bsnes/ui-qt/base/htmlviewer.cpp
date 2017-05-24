#include "htmlviewer.moc"
HtmlViewerWindow *htmlViewerWindow;

HtmlViewerWindow::HtmlViewerWindow() {
  setObjectName("html-window");
  resize(560, 480);
  setGeometryString(&config().geometry.htmlViewerWindow);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setMargin(UIStyle::WindowMargin);
  layout->setSpacing(0);
  setLayout(layout);

  document = new QTextBrowser;
  document->setOpenExternalLinks(true);
  layout->addWidget(document);
}

void HtmlViewerWindow::show(const char *title, const char *htmlData) {
  document->setHtml(string() << htmlData);
  setWindowTitle(title);
  Window::show();
}
