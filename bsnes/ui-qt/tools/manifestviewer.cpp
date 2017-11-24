#include "manifestviewer.moc"
ManifestViewerWindow *manifestViewerWindow;

ManifestViewerWindow::ManifestViewerWindow() {
	layout = new QVBoxLayout;
	layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	layout->setMargin(Style::WindowMargin);
	layout->setSpacing(Style::WidgetSpacing);
	setLayout(layout);

	textEdit = new QPlainTextEdit;
	textEdit->setReadOnly(true);
	textEdit->setFont(QFont(Style::Monospace));
	textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
	layout->addWidget(textEdit);
	
	synchronize();
}

void ManifestViewerWindow::synchronize() {
	if (SNES::cartridge.loaded())
		textEdit->setPlainText(cartridge.baseXml);
	else
		textEdit->setPlainText("No cartridge loaded.");
}
