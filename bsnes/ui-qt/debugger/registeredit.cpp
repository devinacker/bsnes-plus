#include "registeredit.moc"

RegisterEditCPU *registerEditCPU, *registerEditSA1;
RegisterEditSMP *registerEditSMP;
RegisterEditSFX *registerEditSFX;

#define reg_editor(reg, digits) \
	layout->addWidget(new QLabel(QString(#reg).toUpper())); \
	edit_##reg = new QLineEdit(this); \
	edit_##reg->setFont(QFont(Style::Monospace)); \
	edit_##reg->setFixedWidth((digits + 1) * edit_##reg->fontMetrics().width(' ')); \
	edit_##reg->setInputMask(QString("H").repeated(digits)); \
	edit_##reg->setMaxLength(digits); \
	connect(edit_##reg, SIGNAL(textEdited(QString)), this, SLOT(commit())); \
	layout->addWidget(edit_##reg);

#define reg_sync(reg) \
	edit_##reg->setText(QString::number((unsigned) reg, 16).rightJustified(edit_##reg->maxLength(), '0').toUpper());
#define reg_commit(reg) \
	if (sender() == edit_##reg) { \
		bool ok; \
		int val = edit_##reg->text().toInt(&ok, 16); \
		int pos = edit_##reg->cursorPosition(); \
		if (ok) reg = val; \
		synchronize(); \
		edit_##reg->setCursorPosition(pos); \
		return; \
	}

#define flag_editor(flag, num) \
	flag_btn[num] = new QCheckBox(this); \
	flag_btn[num]->setText(QString(flag)); \
	connect(flag_btn[num], SIGNAL(toggled(bool)), this, SLOT(commit())); \
	layout->addWidget(flag_btn[num]);

#define flag_sync(flag, num) flag_btn[num]->setChecked(flag);
#define flag_commit(flag, num) \
	if (sender() == flag_btn[num]) { \
		flag = flag_btn[num]->isChecked(); \
		synchronize(); \
		return; \
	}

void RegisterEditCPU::setupUI() {
	QVBoxLayout *fullLayout = new QVBoxLayout;
	this->setLayout(fullLayout);
	
	QHBoxLayout *layout = new QHBoxLayout;
	reg_editor(pc, 6);
	reg_editor(a, 4);
	reg_editor(x, 4);
	reg_editor(y, 4);
	reg_editor(s, 4);
	reg_editor(d, 4);
	reg_editor(db, 2);
	layout->addStretch();
	fullLayout->addLayout(layout);
	
	layout = new QHBoxLayout;
	reg_editor(p, 2);
	for (int i = 0; i < 9; i++) {
		flag_editor(i["ENVMXDIZC"], i);
	}
	layout->addStretch();
	fullLayout->addLayout(layout);
	
	fullLayout->addStretch();
}

void RegisterEditCPU::commit() {
	reg_commit(pc);
	reg_commit(a);
	reg_commit(x);
	reg_commit(y);
	reg_commit(s);
	reg_commit(d);
	reg_commit(db);
	reg_commit(p);
	
	flag_commit(e, 0);
	flag_commit(p.n, 1);
	flag_commit(p.v, 2);
	flag_commit(p.m, 3);
	flag_commit(p.x, 4);
	flag_commit(p.d, 5);
	flag_commit(p.i, 6);
	flag_commit(p.z, 7);
	flag_commit(p.c, 8);
}

void RegisterEditCPU::synchronize() {
	reg_sync(pc);
	reg_sync(a);
	reg_sync(x);
	reg_sync(y);
	reg_sync(s);
	reg_sync(d);
	reg_sync(db);
	reg_sync(p);
	
	flag_sync(e, 0);
	flag_sync(p.n, 1);
	flag_sync(p.v, 2);
	flag_sync(p.m, 3);
	flag_sync(p.x, 4);
	flag_sync(p.d, 5);
	flag_sync(p.i, 6);
	flag_sync(p.z, 7);
	flag_sync(p.c, 8);
}

void RegisterEditSMP::setupUI() {
	QVBoxLayout *fullLayout = new QVBoxLayout;
	this->setLayout(fullLayout);
	
	QHBoxLayout *layout = new QHBoxLayout;
	reg_editor(pc, 4);
	reg_editor(a, 2);
	reg_editor(x, 2);
	reg_editor(y, 2);
	reg_editor(s, 2);
	reg_editor(ya, 4);
	layout->addStretch();
	fullLayout->addLayout(layout);
	
	layout = new QHBoxLayout;
	reg_editor(p, 2);
	for (int i = 0; i < 8; i++) {
		flag_editor(i["NVPBHIZC"], i);
	}
	layout->addStretch();
	fullLayout->addLayout(layout);
	
	fullLayout->addStretch();
}

void RegisterEditSMP::commit() {
	reg_commit(pc);
	reg_commit(a);
	reg_commit(x);
	reg_commit(y);
	reg_commit(s);
	reg_commit(ya);
	reg_commit(p);
	
	flag_commit(p.n, 0);
	flag_commit(p.v, 1);
	flag_commit(p.p, 2);
	flag_commit(p.b, 3);
	flag_commit(p.h, 4);
	flag_commit(p.i, 5);
	flag_commit(p.z, 6);
	flag_commit(p.c, 7);
}

void RegisterEditSMP::synchronize() {
	reg_sync(pc);
	reg_sync(a);
	reg_sync(x);
	reg_sync(y);
	reg_sync(s);
	reg_sync(ya);
	reg_sync(p);
	
	flag_sync(p.n, 0);
	flag_sync(p.v, 1);
	flag_sync(p.p, 2);
	flag_sync(p.b, 3);
	flag_sync(p.h, 4);
	flag_sync(p.i, 5);
	flag_sync(p.z, 6);
	flag_sync(p.c, 7);
}

void RegisterEditSFX::setupUI() {
	QVBoxLayout *fullLayout = new QVBoxLayout;
	this->setLayout(fullLayout);
	
	QGridLayout *grid = new QGridLayout;
	for (int reg = 0; reg < 16; reg++) {
		grid->addWidget(new QLabel(QString("R%1").arg(reg)), reg / 8, (reg % 8) * 2); \
		edit_r[reg] = new QLineEdit(this); \
		edit_r[reg]->setFont(QFont(Style::Monospace)); \
		edit_r[reg]->setFixedWidth((4 + 1) * edit_r[reg]->fontMetrics().width(' ')); \
		edit_r[reg]->setInputMask("HHHH"); \
		edit_r[reg]->setMaxLength(4); \
		connect(edit_r[reg], SIGNAL(textEdited(QString)), this, SLOT(commit())); \
		grid->addWidget(edit_r[reg], reg / 8, ((reg % 8) * 2) + 1);
	}
	fullLayout->addLayout(grid);
	
	QHBoxLayout *layout = new QHBoxLayout;
	// TODO: some other registers here (ROMBR, etc)
	reg_editor(sfr, 4);
	flag_editor("I", 0);
	flag_editor("B", 1);
	flag_editor("IH", 2);
	flag_editor("IL", 3);
	flag_editor("A2", 4);
	flag_editor("A1", 5);
	flag_editor("R", 6);
	flag_editor("G", 7);
	flag_editor("V", 8);
	flag_editor("N", 9);
	flag_editor("C", 10);
	flag_editor("Z", 11);
	layout->addStretch();
	fullLayout->addLayout(layout);
	
	fullLayout->addStretch();
}

void RegisterEditSFX::commit() {
	for (int i = 0; i < 16; i++) {
		reg_commit(r[i]);
	}
	reg_commit(sfr);
	
	flag_commit(sfr.irq, 0);
	flag_commit(sfr.b, 1);
	flag_commit(sfr.ih, 2);
	flag_commit(sfr.il, 3);
	flag_commit(sfr.alt2, 4);
	flag_commit(sfr.alt1, 5);
	flag_commit(sfr.r, 6);
	flag_commit(sfr.g, 7);
	flag_commit(sfr.ov, 8);
	flag_commit(sfr.s, 9);
	flag_commit(sfr.cy, 10);
	flag_commit(sfr.z, 11);
}

void RegisterEditSFX::synchronize() {
	for (int i = 0; i < 16; i++) {
		reg_sync(r[i]);
	}
	reg_sync(sfr);
	
	flag_sync(sfr.irq, 0);
	flag_sync(sfr.b, 1);
	flag_sync(sfr.ih, 2);
	flag_sync(sfr.il, 3);
	flag_sync(sfr.alt2, 4);
	flag_sync(sfr.alt1, 5);
	flag_sync(sfr.r, 6);
	flag_sync(sfr.g, 7);
	flag_sync(sfr.ov, 8);
	flag_sync(sfr.s, 9);
	flag_sync(sfr.cy, 10);
	flag_sync(sfr.z, 11);
}

#undef reg_editor
#undef reg_sync
#undef reg_commit
#undef flag_editor
#undef flag_sync
#undef flag_commit
