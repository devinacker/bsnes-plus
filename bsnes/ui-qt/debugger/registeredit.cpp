#include "registeredit.moc"

RegisterEditCPU *registerEditCPU, *registerEditSA1;
RegisterEditSMP *registerEditSMP;
RegisterEditSFX *registerEditSFX;

#define reg_editor(name, digits) \
	layout->addWidget(new QLabel(QString(#name).toUpper()), layout->rowCount(), 0); \
	edit_##name = new QLineEdit(this); \
	edit_##name->setFont(QFont(Style::Monospace)); \
	edit_##name->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum); \
	edit_##name->setFixedWidth((digits + 1) * edit_##name->fontMetrics().width(' ')); \
	edit_##name->setInputMask(QString("H").repeated(digits)); \
	edit_##name->setMaxLength(digits); \
	connect(edit_##name, SIGNAL(textEdited(QString)), this, SLOT(commit())); \
	layout->addWidget(edit_##name, layout->rowCount() - 1, 1);

#define reg_sync(name, reg) \
	edit_##name->setText(QString::number(_debugger.getRegister(reg), 16) \
		.rightJustified(edit_##name->maxLength(), '0').toUpper());
#define reg_commit(name, reg) \
	if (sender() == edit_##name) { \
		bool ok; \
		int val = edit_##name->text().toInt(&ok, 16); \
		int pos = edit_##name->cursorPosition(); \
		if (ok) _debugger.setRegister(reg, val); \
		synchronize(); \
		edit_##name->setCursorPosition(pos); \
		return; \
	}

#define flag_editor(flag, num, row, column) \
	flag_btn[num] = new QCheckBox(this); \
	flag_btn[num]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); \
	flag_btn[num]->setText(QString(flag)); \
	connect(flag_btn[num], SIGNAL(toggled(bool)), this, SLOT(commit())); \
	layout->addWidget(flag_btn[num], row, column);

#define flag_sync(flag, num) \
	flag_btn[num]->blockSignals(true); \
	flag_btn[num]->setChecked(_debugger.getFlag(flag)); \
	flag_btn[num]->blockSignals(false);
#define flag_commit(flag, num) \
	if (sender() == flag_btn[num]) { \
		_debugger.setFlag(flag, flag_btn[num]->isChecked()); \
		synchronize(); \
		return; \
	}

void RegisterEditCPU::setupUI() {
	QVBoxLayout *fullLayout = new QVBoxLayout;
	this->setLayout(fullLayout);

	QGridLayout *layout = new QGridLayout;
	reg_editor(pc, 6);
	reg_editor(a, 4);
	reg_editor(x, 4);
	reg_editor(y, 4);
	reg_editor(s, 4);
	reg_editor(d, 4);
	reg_editor(db, 2);
	reg_editor(p, 2);
	fullLayout->addLayout(layout);

	layout = new QGridLayout;
	flag_editor("E", 0, 0, 0);
	for (int i = 0; i < 8; i++) {
		flag_editor(i["NVMXDIZC"], i + 1, (i >> 1) + 1, i & 1);
	}
	fullLayout->addLayout(layout);

	fullLayout->addStretch();
}

void RegisterEditCPU::commit() {
	reg_commit(pc, SNES::CPUDebugger::RegisterPC);
	reg_commit(a,  SNES::CPUDebugger::RegisterA);
	reg_commit(x,  SNES::CPUDebugger::RegisterX);
	reg_commit(y,  SNES::CPUDebugger::RegisterY);
	reg_commit(s,  SNES::CPUDebugger::RegisterS);
	reg_commit(d,  SNES::CPUDebugger::RegisterD);
	reg_commit(db, SNES::CPUDebugger::RegisterDB);
	reg_commit(p,  SNES::CPUDebugger::RegisterP);
	
	flag_commit(SNES::CPUDebugger::FlagE, 0);
	flag_commit(SNES::CPUDebugger::FlagN, 1);
	flag_commit(SNES::CPUDebugger::FlagV, 2);
	flag_commit(SNES::CPUDebugger::FlagM, 3);
	flag_commit(SNES::CPUDebugger::FlagX, 4);
	flag_commit(SNES::CPUDebugger::FlagD, 5);
	flag_commit(SNES::CPUDebugger::FlagI, 6);
	flag_commit(SNES::CPUDebugger::FlagZ, 7);
	flag_commit(SNES::CPUDebugger::FlagC, 8);
}

void RegisterEditCPU::synchronize() {
	reg_sync(pc, SNES::CPUDebugger::RegisterPC);
	reg_sync(a,  SNES::CPUDebugger::RegisterA);
	reg_sync(x,  SNES::CPUDebugger::RegisterX);
	reg_sync(y,  SNES::CPUDebugger::RegisterY);
	reg_sync(s,  SNES::CPUDebugger::RegisterS);
	reg_sync(d,  SNES::CPUDebugger::RegisterD);
	reg_sync(db, SNES::CPUDebugger::RegisterDB);
	reg_sync(p,  SNES::CPUDebugger::RegisterP);
	
	flag_sync(SNES::CPUDebugger::FlagE, 0);
	flag_sync(SNES::CPUDebugger::FlagN, 1);
	flag_sync(SNES::CPUDebugger::FlagV, 2);
	flag_sync(SNES::CPUDebugger::FlagM, 3);
	flag_sync(SNES::CPUDebugger::FlagX, 4);
	flag_sync(SNES::CPUDebugger::FlagD, 5);
	flag_sync(SNES::CPUDebugger::FlagI, 6);
	flag_sync(SNES::CPUDebugger::FlagZ, 7);
	flag_sync(SNES::CPUDebugger::FlagC, 8);
}

void RegisterEditSMP::setupUI() {
	QVBoxLayout *fullLayout = new QVBoxLayout;
	this->setLayout(fullLayout);

	QGridLayout *layout = new QGridLayout;
	reg_editor(pc, 4);
	reg_editor(a, 2);
	reg_editor(x, 2);
	reg_editor(y, 2);
	reg_editor(s, 2);
	reg_editor(ya, 4);
	reg_editor(p, 2);
	fullLayout->addLayout(layout);

	layout = new QGridLayout;
	for (int i = 0; i < 8; i++) {
		flag_editor(i["NVPBHIZC"], i, i >> 1, i & 1);
	}
	fullLayout->addLayout(layout);

	fullLayout->addStretch();
}

void RegisterEditSMP::commit() {
	reg_commit(pc, SNES::SMPDebugger::RegisterPC);
	reg_commit(a,  SNES::SMPDebugger::RegisterA);
	reg_commit(x,  SNES::SMPDebugger::RegisterX);
	reg_commit(y,  SNES::SMPDebugger::RegisterY);
	reg_commit(s,  SNES::SMPDebugger::RegisterS);
	reg_commit(ya, SNES::SMPDebugger::RegisterYA);
	reg_commit(p,  SNES::SMPDebugger::RegisterP);
	
	flag_commit(SNES::SMPDebugger::FlagN, 0);
	flag_commit(SNES::SMPDebugger::FlagV, 1);
	flag_commit(SNES::SMPDebugger::FlagP, 2);
	flag_commit(SNES::SMPDebugger::FlagB, 3);
	flag_commit(SNES::SMPDebugger::FlagH, 4);
	flag_commit(SNES::SMPDebugger::FlagI, 5);
	flag_commit(SNES::SMPDebugger::FlagZ, 6);
	flag_commit(SNES::SMPDebugger::FlagC, 7);
}

void RegisterEditSMP::synchronize() {
	reg_sync(pc, SNES::SMPDebugger::RegisterPC);
	reg_sync(a,  SNES::SMPDebugger::RegisterA);
	reg_sync(x,  SNES::SMPDebugger::RegisterX);
	reg_sync(y,  SNES::SMPDebugger::RegisterY);
	reg_sync(s,  SNES::SMPDebugger::RegisterS);
	reg_sync(ya, SNES::SMPDebugger::RegisterYA);
	reg_sync(p,  SNES::SMPDebugger::RegisterP);

	flag_sync(SNES::SMPDebugger::FlagN, 0);
	flag_sync(SNES::SMPDebugger::FlagV, 1);
	flag_sync(SNES::SMPDebugger::FlagP, 2);
	flag_sync(SNES::SMPDebugger::FlagB, 3);
	flag_sync(SNES::SMPDebugger::FlagH, 4);
	flag_sync(SNES::SMPDebugger::FlagI, 5);
	flag_sync(SNES::SMPDebugger::FlagZ, 6);
	flag_sync(SNES::SMPDebugger::FlagC, 7);
}

void RegisterEditSFX::setupUI() {
	QVBoxLayout *fullLayout = new QVBoxLayout;
	this->setLayout(fullLayout);

	QGridLayout *layout = new QGridLayout;
	for (int reg = 0; reg < 16; reg++) {
		layout->addWidget(new QLabel(QString("R%1").arg(reg)), reg>>1, (reg&1)<<1);
		edit_r[reg] = new QLineEdit(this);
		edit_r[reg]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
		edit_r[reg]->setFont(QFont(Style::Monospace));
		edit_r[reg]->setFixedWidth((4 + 1) * edit_r[reg]->fontMetrics().width(' '));
		edit_r[reg]->setInputMask("HHHH");
		edit_r[reg]->setMaxLength(4);
		connect(edit_r[reg], SIGNAL(textEdited(QString)), this, SLOT(commit()));
		layout->addWidget(edit_r[reg], reg>>1, ((reg&1)<<1) + 1);
	}
	reg_editor(sfr, 4);
	fullLayout->addLayout(layout);

	layout = new QGridLayout;
	// TODO: some other registers here (ROMBR, etc)
	flag_editor("I", 0, 0, 0);
	flag_editor("B", 1, 0, 1);
	flag_editor("IH", 2, 1, 0);
	flag_editor("IL", 3, 1, 1);
	flag_editor("A2", 4, 2, 0);
	flag_editor("A1", 5, 2, 1);
	flag_editor("R", 6, 3, 0);
	flag_editor("G", 7, 3, 1);
	flag_editor("V", 8, 4, 0);
	flag_editor("N", 9, 4, 1);
	flag_editor("C", 10, 5, 0);
	flag_editor("Z", 11, 5, 1);
	//layout->addStretch();
	fullLayout->addLayout(layout);
}

void RegisterEditSFX::commit() {
	for (int i = 0; i < 16; i++) {
		reg_commit(r[i], i);
	}
	reg_commit(sfr, SNES::SFXDebugger::RegisterSFR);
	
	flag_commit(SNES::SFXDebugger::FlagI, 0);
	flag_commit(SNES::SFXDebugger::FlagB, 1);
	flag_commit(SNES::SFXDebugger::FlagIH, 2);
	flag_commit(SNES::SFXDebugger::FlagIL, 3);
	flag_commit(SNES::SFXDebugger::FlagA2, 4);
	flag_commit(SNES::SFXDebugger::FlagA1, 5);
	flag_commit(SNES::SFXDebugger::FlagR, 6);
	flag_commit(SNES::SFXDebugger::FlagG, 7);
	flag_commit(SNES::SFXDebugger::FlagV, 8);
	flag_commit(SNES::SFXDebugger::FlagN, 9);
	flag_commit(SNES::SFXDebugger::FlagC, 10);
	flag_commit(SNES::SFXDebugger::FlagZ, 11);
}

void RegisterEditSFX::synchronize() {
	for (int i = 0; i < 16; i++) {
		reg_sync(r[i], i);
	}
	reg_sync(sfr, SNES::SFXDebugger::RegisterSFR);
	
	flag_sync(SNES::SFXDebugger::FlagI, 0);
	flag_sync(SNES::SFXDebugger::FlagB, 1);
	flag_sync(SNES::SFXDebugger::FlagIH, 2);
	flag_sync(SNES::SFXDebugger::FlagIL, 3);
	flag_sync(SNES::SFXDebugger::FlagA2, 4);
	flag_sync(SNES::SFXDebugger::FlagA1, 5);
	flag_sync(SNES::SFXDebugger::FlagR, 6);
	flag_sync(SNES::SFXDebugger::FlagG, 7);
	flag_sync(SNES::SFXDebugger::FlagV, 8);
	flag_sync(SNES::SFXDebugger::FlagN, 9);
	flag_sync(SNES::SFXDebugger::FlagC, 10);
	flag_sync(SNES::SFXDebugger::FlagZ, 11);
}

#undef reg_editor
#undef reg_sync
#undef reg_commit
#undef flag_editor
#undef flag_sync
#undef flag_commit
