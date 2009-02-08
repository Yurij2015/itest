/*******************************************************************
 This file is part of iTest
 Copyright (C) 2007 Michal Tomlein (michal.tomlein@gmail.com)

 iTest is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 iTest is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with iTest; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
********************************************************************/

#include "main_window.h"

void MainWindow::setupServer()
{
    tcpServer = new QTcpServer(this);
    QObject::connect(mainStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(enableSMTools()));
    QObject::connect(tcpServer, SIGNAL(newConnection()), this, SLOT(addClient()));
    QObject::connect(btnStartServer, SIGNAL(released()), this, SLOT(startServer()));
    QObject::connect(actionStart_server, SIGNAL(triggered()), this, SLOT(startServer()));
    QObject::connect(actionStop_server, SIGNAL(triggered()), this, SLOT(stopServer()));
    QObject::connect(actionRun_Test_Writer, SIGNAL(triggered()), this, SLOT(runTestWriter()));
    QObject::connect(actionExport_log, SIGNAL(triggered()), this, SLOT(exportLog()));
    QObject::connect(SMLCListWidget, SIGNAL(currentTextChanged(QString)), this, SLOT(setCurrentClient()));
    QObject::connect(actionQuickPrint, SIGNAL(triggered()), this, SLOT(quickPrint()));
    QObject::connect(actionPrint, SIGNAL(triggered()), this, SLOT(print()));
    QObject::connect(actionExport_test, SIGNAL(triggered()), this, SLOT(exportTest()));
    QObject::connect(actionAdd_offline_client, SIGNAL(triggered()), this, SLOT(addOfflineClient()));
    QObject::connect(actionAdd_offline_clients, SIGNAL(triggered()), this, SLOT(addOfflineClients()));
    enableSMTools(); togglePrintEnabled();
    
    rbtngrpAdvSelect = new QButtonGroup (this);
    rbtngrpAdvSelect->addButton(rbtnSelectFlags);
    rbtngrpAdvSelect->addButton(rbtnSelectQuestions);
    QObject::connect(rbtngrpAdvSelect, SIGNAL(buttonReleased(QAbstractButton *)), this, SLOT(reloadAvailableItems()));
    QObject::connect(TSAdvancedSetupGroupBox, SIGNAL(toggled(bool)), this, SLOT(reloadAvailableItems()));
    QObject::connect(TSSearchAvailableLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(searchAvailableItems(const QString &)));
    QObject::connect(TSSearchUsedLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(searchUsedItems(const QString &)));
    QObject::connect(TSExcludeListWidget, SIGNAL(currentTextChanged(QString)), this, SLOT(toggleAddRemoveEnabled()));
    QObject::connect(TSIncludeTableWidget, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)), this, SLOT(toggleAddRemoveEnabled()));
    QObject::connect(btnAddToList, SIGNAL(released()), this, SLOT(addToList()));
    QObject::connect(btnRemoveFromList, SIGNAL(released()), this, SLOT(removeFromList()));
    QObject::connect(TSExcludeListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(addToList()));
    QObject::connect(TSIncludeTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(removeFromList()));
    TSExcludeListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    //TSIncludeListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    TSIncludeTableWidget->verticalHeader()->hide();
    TSIncludeTableWidget->horizontalHeader()->hide();
    TSIncludeTableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    QObject::connect(TSMaxQnumCheckBox, SIGNAL(toggled(bool)), this, SLOT(setMaximumQuestions()));
    QObject::connect(TSGroupsCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateTestQnum()));
    QObject::connect(TSQnumSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTestTime()));
    QObject::connect(TSQnumSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMaximumPassMark(int)));
    QObject::connect(rbtnTestTime, SIGNAL(toggled(bool)), this, SLOT(updateTestTime()));
    QObject::connect(rbtnQuestionTime, SIGNAL(toggled(bool)), this, SLOT(updateTestTime()));
    QObject::connect(TSTestTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateTestTime()));
    QObject::connect(TSQuestionTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateTestTime()));
}

void MainWindow::setMaximumPassMark(int value)
{
	TSPassMarkSpinBox->setMaximum(value);
}

void MainWindow::toggleAddRemoveEnabled()
{
    if (TSExcludeListWidget->currentIndex().isValid()) {
        btnAddToList->setEnabled(true);
    } else { btnAddToList->setEnabled(false); }
    if (TSIncludeTableWidget->currentRow() >= 0) {
        btnRemoveFromList->setEnabled(true);
    } else { btnRemoveFromList->setEnabled(false); }
}

void MainWindow::reloadAvailableItems()
{
	TSExcludeListWidget->clear(); TSIncludeTableWidget->clear();
	TSIncludeTableWidget->setRowCount(0);
	TSSearchAvailableLineEdit->clear(); TSSearchUsedLineEdit->clear();
	if (!TSAdvancedSetupGroupBox->isChecked()) {
        TSIncludeTableWidget->horizontalHeader()->hide();
    } else {
        if (rbtnSelectFlags->isChecked()) {
            // FLAGS -----------------------------------------------------------
            TSIncludeTableWidget->setColumnCount(2);
            TSIncludeTableWidget->horizontalHeader()->show();
            TSIncludeTableWidget->setHorizontalHeaderLabels(QStringList() << tr("Flag name") << tr("Pass mark"));
            QListWidgetItem * item;
            for (int i = 0; i < 20; ++i) {
                if (current_db_fe[i]) {
                    item = new QListWidgetItem (QString("%1 - %2").arg(i+1).arg(current_db_f[i]), TSExcludeListWidget);
                    item->setData(Qt::UserRole, i);
                    setQuestionItemColour(item, i);
                }
            }
            toggleAddRemoveEnabled();
            // -----------------------------------------------------------------
        } else if (rbtnSelectQuestions->isChecked()) {
            // QUESTIONS -------------------------------------------------------
            TSIncludeTableWidget->setColumnCount(1);
            TSIncludeTableWidget->horizontalHeader()->hide();
            QListWidgetItem * item; QuestionItem * q_item;
            for (int i = 0; i < LQListWidget->count(); ++i) {
                item = new QListWidgetItem (*LQListWidget->item(i));
                q_item = NULL; q_item = current_db_questions.value(LQListWidget->item(i));
                if (q_item != NULL) { if (q_item->isHidden()) { continue; } }
                item->setData(Qt::UserRole, i);
                TSExcludeListWidget->addItem(item);
            }
            toggleAddRemoveEnabled();
            // -----------------------------------------------------------------
        }
    }
    updateTestQnum();
}

void MainWindow::updateTestQnum()
{
    int db_qnum = 0; int min_qnum = 0; QuestionItem * item;
    if (!TSAdvancedSetupGroupBox->isChecked()) {
    	if (TSGroupsCheckBox->isChecked()) {
    		QSet<QString> groups;
    		for (int q = 0; q < LQListWidget->count(); ++q) {
    			item = NULL; item = current_db_questions.value(LQListWidget->item(q));
    			if (item == NULL) { continue; }
    			if (item->isHidden()) { continue; }
    			if (item->group().isEmpty()) { db_qnum++; }
    			else { groups << item->group(); }
    		}
    		db_qnum += groups.count();
    	} else {
    		for (int q = 0; q < LQListWidget->count(); ++q) {
    			item = NULL; item = current_db_questions.value(LQListWidget->item(q));
    			if (item == NULL) { continue; }
    		    if (item->isHidden()) { continue; }
    		    db_qnum++;
    		}
    	}
    } else if (rbtnSelectFlags->isChecked()) {
    	QSet<QString> groups; QList<int> used_items;
    	int max[TSIncludeTableWidget->rowCount()]; QSet<QString> groups_i[TSIncludeTableWidget->rowCount()];
        for (int i = 0; i < TSIncludeTableWidget->rowCount(); ++i) {
            used_items << TSIncludeTableWidget->item(i, 0)->data(Qt::UserRole).toInt();
            max[i] = 0;
        }
        for (int i = 0; i < LQListWidget->count(); ++i) {
        	item = NULL; item = current_db_questions.value(LQListWidget->item(i));
        	if (item == NULL) { continue; }
            if (item->isHidden()) { continue; }
            if (used_items.contains(item->flag())) {
            	if (TSGroupsCheckBox->isChecked()) {
            		if (item->group().isEmpty()) { db_qnum++; max[used_items.indexOf(item->flag())]++; }
            		else { groups << item->group(); groups_i[used_items.indexOf(item->flag())] << item->group(); }
            	} else { db_qnum++; max[used_items.indexOf(item->flag())]++; }
            }
        }
        db_qnum += groups.count();
        for (int i = 0; i < TSIncludeTableWidget->rowCount(); ++i) {
        	max[i] += groups_i[i].count();
        	((QSpinBox *)TSIncludeTableWidget->cellWidget(i, 1))->setMaximum(max[i]);
        	min_qnum += ((QSpinBox *)TSIncludeTableWidget->cellWidget(i, 1))->value();
        }
    } else if (rbtnSelectQuestions->isChecked()) {
    	if (TSGroupsCheckBox->isChecked()) {
    		QSet<QString> groups;
    		for (int q = 0; q < TSIncludeTableWidget->rowCount(); ++q) {
    			item = NULL; item = current_db_questions.value(LQListWidget->item(TSIncludeTableWidget->item(q, 0)->data(Qt::UserRole).toInt()));
    			if (item == NULL) { continue; }
    			if (item->group().isEmpty()) { db_qnum++; }
    	    	else { groups << item->group(); }
    		}
    	    db_qnum += groups.count();
    	} else {
    		db_qnum = TSIncludeTableWidget->rowCount();
    	}
    }
    if (min_qnum <= db_qnum) {
    	TSQnumSpinBox->setMinimum(min_qnum);
    	TSPassMarkSpinBox->setMinimum(min_qnum);
    } else {
    	TSQnumSpinBox->setMinimum(0);
    	TSPassMarkSpinBox->setMinimum(0);
    }
    if (db_qnum < 1) {
        TSQnumSpinBox->setMaximum(0);
        TSMaxQnumCheckBox->setText(tr("maximum (%1)").arg(db_qnum));
        btnStartServer->setEnabled(false);
        actionStart_server->setEnabled(false);
    } else {
        TSQnumSpinBox->setMaximum(db_qnum);
        TSMaxQnumCheckBox->setText(tr("maximum (%1)").arg(db_qnum));
        if (TSMaxQnumCheckBox->isChecked()) {
            TSQnumSpinBox->setValue(db_qnum);
        }
        btnStartServer->setEnabled(true);
        actionStart_server->setEnabled(true);
    }
}

void MainWindow::updateTestTime()
{
	if (rbtnTestTime->isChecked()) {
		if (TSQnumSpinBox->value() == 0) {
			TSQuestionTimeEdit->setTime(QTime::QTime(0, 0));
		} else {
			QTime time = TSTestTimeEdit->time(); QTime nulltime (0, 0);
			QTime qtime = nulltime.addSecs(((time.hour() * 60 * 60) + (time.minute() * 60)) / TSQnumSpinBox->value());
			TSQuestionTimeEdit->setTime(qtime);
		}
	} else if (rbtnQuestionTime->isChecked()) {
		QTime time = TSQuestionTimeEdit->time(); QTime nulltime (0, 0);
		while ((((time.hour() * 60 * 60) + (time.minute() * 60)) * TSQnumSpinBox->value()) > 86340) {
			time = time.addSecs(-60); qApp->processEvents();
		}
		TSQuestionTimeEdit->setTime(time);
		QTime qtime = nulltime.addSecs(((time.hour() * 60 * 60) + (time.minute() * 60)) * TSQnumSpinBox->value());
		TSTestTimeEdit->setTime(qtime);
	}
}

void MainWindow::setMaximumQuestions()
{
	TSQnumSpinBox->setValue(TSQnumSpinBox->maximum());
}

void MainWindow::searchAvailableItems(const QString & keyword)
{
	searchListWidgetItems(keyword, TSExcludeListWidget, TSSearchAvailableLineEdit);
}

void MainWindow::searchUsedItems(const QString & keyword)
{
	searchTableWidgetItems(keyword, TSIncludeTableWidget, TSSearchUsedLineEdit);
}

void MainWindow::addToList()
{
    if (TSExcludeListWidget->currentIndex().isValid()) {
    	TSIncludeTableWidget->setRowCount(TSIncludeTableWidget->rowCount() + 1);
    	QListWidgetItem * lw_item = TSExcludeListWidget->takeItem(TSExcludeListWidget->currentRow());
    	QTableWidgetItem * item = new QTableWidgetItem(lw_item->icon(), lw_item->text());
    	item->setBackground(lw_item->background());
    	item->setForeground(lw_item->foreground());
    	item->setData(Qt::UserRole, lw_item->data(Qt::UserRole));
    	TSIncludeTableWidget->setItem(TSIncludeTableWidget->rowCount() - 1, 0, item);
    	if (rbtnSelectFlags->isChecked()) {
    		QSpinBox * spinbox = new QSpinBox(this);
    		QuestionItem * qi; int max = 0; QSet<QString> groups;
    		for (int i = 0; i < LQListWidget->count(); ++i) {
    			qi = current_db_questions.value(LQListWidget->item(i));
    			if (item->data(Qt::UserRole).toInt() == qi->flag()) {
    				if (TSGroupsCheckBox->isChecked()) {
    					if (qi->group().isEmpty()) { max++; }
    					else { groups << qi->group(); }
    				} else { max++; }
    			}
    		}
    		max += groups.count();
    		spinbox->setMaximum(max);
    		spinbox->setFixedHeight(20);
    		QObject::connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(updateTestQnum()));
    		TSIncludeTableWidget->setCellWidget(TSIncludeTableWidget->rowCount() - 1, 1, spinbox);
    		TSIncludeTableWidget->setRowHeight(TSIncludeTableWidget->rowCount() - 1, 20);
    	} else {
    		TSIncludeTableWidget->setRowHeight(TSIncludeTableWidget->rowCount() - 1, 16);
    	}
    	delete lw_item;
        updateTestQnum();
        toggleAddRemoveEnabled();
    }
}

void MainWindow::removeFromList()
{
    if (TSIncludeTableWidget->currentRow() >= 0) {
    	QTableWidgetItem * item = TSIncludeTableWidget->takeItem(TSIncludeTableWidget->currentRow(), 0);
    	QListWidgetItem * lw_item = new QListWidgetItem(item->icon(), item->text());
    	lw_item->setBackground(item->background());
    	lw_item->setForeground(item->foreground());
    	lw_item->setData(Qt::UserRole, item->data(Qt::UserRole));
        TSExcludeListWidget->addItem(lw_item);
        TSIncludeTableWidget->removeRow(TSIncludeTableWidget->currentRow());
        updateTestQnum();
        toggleAddRemoveEnabled();
    }
}

void MainWindow::startServer()
{
    if (this->isWindowModified()) {
        switch (QMessageBox::information(this, tr("iTestServer"), tr("It is necessary to save any changes you have made to the database before proceeding."), tr("&Save"), tr("Cancel"), 0, 1)) {
            case 0: // Save
                save(); break;
            case 1: // Cancel
                return; break;
        }
    }
    
	if (TSCustomTestNameCheckBox->isChecked() && TSTestNameLineEdit->text().isEmpty())
		{ QMessageBox::critical(this, tr("iTestServer"), tr("Invalid test name.")); return; }
    QTime time = TSTestTimeEdit->time(); if (!time.isValid())
		{ QMessageBox::critical(this, tr("iTestServer"), tr("Invalid exam time.")); return; }
    if ((time.hour() == 0) && (time.minute() == 0) && (time.second() == 0))
		{ QMessageBox::critical(this, tr("iTestServer"), tr("Students will need at least one minute for the exam,\nalthough it is recommended to give them an hour.")); return; }
    
    if (!tcpServer->listen(QHostAddress::Any, TSCustomServerPortCheckBox->isChecked() ? TSCustomServerPortSpinBox->value() : 0) && !tcpServer->isListening()) {
        QMessageBox::critical(this, tr("iTestServer"), tr("Unable to start the server: %1.")
                         .arg(tcpServer->errorString()));
        tcpServer->close();
        return;
    }
    
    setProgress(2); // PROGRESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    
    if (!loadPrinterSettings()) {
        switch (QMessageBox::information(this, tr("iTestServer"), tr("You have not configured the printer yet. Would you like to configure it now?"), tr("Con&figure"), tr("Cancel"), 0, 0)) {
            case 0: // Configure
                if (!configurePrinter(false)) { setProgress(-1); return; } break;
            case 1: // Cancel
                setProgress(-1); return; break;
        }
    } else {
        QString printer_config;
        if (printerConfiguration(printer_config)) {
            switch (QMessageBox::information(this, tr("iTestServer"), tr("%1Current printer configuration:%2Would you like to review the configuration?%3").arg("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body><p>").arg(printer_config).arg("</p></body></html>"), tr("&Review configuration"), tr("&Continue"), 0, 0)) {
                case 0: // Review configuration
                    if (!configurePrinter(true)) { setProgress(-1); return; } break;
                case 1: // Continue
                    if (!loadPrinterConfiguration()) { setProgress(-1); return; } break;
            }
        } else {
            QMessageBox::critical(this, tr("iTestServer"), tr("Unable to start the server: Invalid printer configuration.")); setProgress(-1); return;
        }
    }
    
    setProgress(4); // PROGRESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    
    // TEST_QNUM
    int test_qnum = TSQnumSpinBox->value();
    // CUSTOM TEST NAME
	if (TSCustomTestNameCheckBox->isChecked()) {
		current_db_testname = TSTestNameLineEdit->text();
	} else { current_db_testname = current_db_name; }
	// TEST DATE
	current_db_testdate = QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm");
	// PASS MARK
	current_db_passmark.clear();
	current_db_passmark.setPassMark(TSPassMarkSpinBox->value());
	if (TSAdvancedSetupGroupBox->isChecked() && rbtnSelectFlags->isChecked()) {
		for (int i = 0; i < TSIncludeTableWidget->rowCount(); ++i) {
			if (((QSpinBox *)TSIncludeTableWidget->cellWidget(i, 1))->value() > 0) {
				current_db_passmark.addCondition(TSIncludeTableWidget->item(i, 0)->data(Qt::UserRole).toInt(), ((QSpinBox *)TSIncludeTableWidget->cellWidget(i, 1))->value());
			}
		}
	}
	// ADVANCED SETUP
    QList<int> used_items; int db_qnum = 0; QuestionItem * item;
    for (int i = 0; i < TSIncludeTableWidget->rowCount(); ++i) {
        used_items << TSIncludeTableWidget->item(i, 0)->data(Qt::UserRole).toInt();
    }
    if (!TSAdvancedSetupGroupBox->isChecked()) {
    	for (int q = 0; q < LQListWidget->count(); ++q) {
    	    item = NULL; item = current_db_questions.value(LQListWidget->item(q));
    	    if (item == NULL) { continue; }
    	    if (item->isHidden()) { continue; }
    	    db_qnum++;
    	}
    } else if (rbtnSelectFlags->isChecked()) {
        for (int i = 0; i < LQListWidget->count(); ++i) {
        	item = NULL; item = current_db_questions.value(LQListWidget->item(i));
        	if (item == NULL) { continue; }
        	if (item->isHidden()) { continue; }
            if (used_items.contains(item->flag())) { db_qnum++; }
        }
    } else if (rbtnSelectQuestions->isChecked()) {
        db_qnum = TSIncludeTableWidget->rowCount();
    } else {
        QMessageBox::critical(this, tr("iTestServer"), tr("Unable to start the server: Invalid server settings.")); setProgress(-1); return;
    }
    // -------------------------------------------------------------------------
    current_db_test.clear(); QByteArray ba; bool ok;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << (quint64)0;
    // -------------------------------------------------------------------------
    setProgress(5); // PROGRESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    out << QString("[ITEST_VERSION]\n") << QString("%1\n").arg(f_ver);
    out << QString("[ITEST_DB_VERSION]\n") << QString("%1\n").arg(f_itos_ver);
    out << QString("[DB_NAME]\n") << current_db_testname << QString("\n");
    out << QString("[DB_DATE]\n") << current_db_date << QString("\n");
    out << QString("[DB_DATE_ULSD]\n") << (actionUse_last_save_date->isChecked() ? QString("true\n") : QString("false\n"));
    out << QString("[TEST_GRPS]\n") << (TSGroupsCheckBox->isChecked() ? QString("true\n") : QString("false\n"));
    out << QString("[TEST_DATE]\n") << current_db_testdate << QString("\n");
    out << QString("[TEST_TIME]\n") << time.toString("HH:mm") << QString("\n");
    out << QString("[TEST_QNUM]\n") << QString("%1\n").arg(test_qnum);
    out << QString("[DB_COMMENTS]\n") << current_db_comments << QString("\n");
    out << QString("[DB_QNUM]\n") << QString("%1\n").arg(db_qnum);
    out << QString("[DB_FLAGS]\n");
    for (int i = 0; i < 20; ++i)
    {if (current_db_fe[i]) {out << QString("+");} else {out << QString("-");}}
    out << QString("\n");
    for (int i = 0; i < 20; ++i)
    {out << QString("[DB_F") << QString("%1").arg(i) << QString("]\n"); out << current_db_f[i] << QString("\n");}
    out << QString("[DB_FLAGS_END]\n");
    out << current_db_passmark.data() << QString("\n");
    setProgress(10); // PROGRESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // -------------------------------------------------------------------------
    out.device()->seek(0);
    out << (quint64)(ba.size() - sizeof(quint64));
    current_db_test << ba;
    for (int i = 0; i < LQListWidget->count(); ++i) {
        ba.clear(); ok = false;
        // ---------------------------------------------------------------------
        item = NULL; item = current_db_questions.value(LQListWidget->item(i));
        if (item != NULL) {
        	if (!TSAdvancedSetupGroupBox->isChecked()) {
        		if (!item->isHidden()) {
        			ok = true;
        		}
        	} else if (rbtnSelectFlags->isChecked()) {
        		if (!item->isHidden()) {
        			if (used_items.contains(item->flag())) {
        				ok = true;
        			}
        		}
        	} else if (rbtnSelectQuestions->isChecked()) {
        		if (used_items.contains(i)) {
        			ok = true;
        		}
        	}
        }
        // ---------------------------------------------------------------------
        if (ok) {
        	out << (quint64)0;
        	out << item->allPublicProperties() << QString("\n");
        	out.device()->seek(0);
        	out << (quint64)(ba.size() - sizeof(quint64));
        	current_db_test << ba;
        	for (int s = 0; s < item->numSvgItems(); ++s) {
        	    ba.clear();
        	    out << (quint64)0;
        	    out << item->svgItem(s)->text() << QString("\n");
        	    out << item->svgItem(s)->svg() << QString("\n");
        	    out.device()->seek(0);
        	    out << (quint64)(ba.size() - sizeof(quint64));
        	    current_db_test << ba;
        	}
        }
        setProgress(((90/current_db_questions.size())*(i+1))+10); // PROGRESS >>
    }
    
    disableAll(); actionQuit->setEnabled(false);
    actionNew->setEnabled(false); actionOpen->setEnabled(false);
    actionSave_session->setChecked(true);
    mainStackedWidget->setCurrentIndex(5);

    SMIPortLabel->setText(tr("The server is running on port <b>%1</b>. "
                       "You may now run iTestClient on each client PC. "
                       "Do not quit iTestServer while testing!")
                       .arg(tcpServer->serverPort()));
    
    setProgress(100); setProgress(-1); // PROGRESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    statusBar()->showMessage("Server ready", 10000);
}

void MainWindow::stopServer()
{
	switch (QMessageBox::information(this, tr("iTestServer - Stop server"), tr("Are you sure you want to stop the server?"), tr("&Stop"), tr("Cancel"), 0, 1)) {
		case 0: // Stop
			break;
		case 1: // Cancel
			return; break;
	}
	tcpServer->close();
	// -------------------------------------------------------------------------
	if (actionSave_session->isChecked() && (numIdentifiedClients() > 0)) {
		Session * session = new Session;
		session->setName(current_db_testname);
		session->setDateTimeFromString(current_db_testdate);
		session->setPassMark(current_db_passmark);
		current_db_passmark.clear();
		for (int i = 0; i < SMSLListWidget->count(); ++i) {
			session->addLogEntry(SMSLListWidget->item(i)->background().color().red(),
									SMSLListWidget->item(i)->background().color().green(),
									SMSLListWidget->item(i)->background().color().blue(),
									SMSLListWidget->item(i)->foreground().color().red(),
									SMSLListWidget->item(i)->foreground().color().green(),
									SMSLListWidget->item(i)->foreground().color().blue(),
									SMSLListWidget->item(i)->text());
		}
		for (int i = 0; i < SMLCListWidget->count(); ++i) {
			if (SMLCListWidget->item(i)->isHidden()) continue;
			Student * student = new Student (current_db_clients.value(SMLCListWidget->item(i)));
			session->addStudent(student);
		}
		current_db_sessions.insert(session->dateTime(), session);
		QListWidgetItem * item = new QListWidgetItem (QString("%1 - %2").arg(session->dateTimeToString()).arg(session->name()));
		VSSLSListWidget->insertItem(0, item);
        item->setData(Qt::UserRole, session->dateTime());
	}
	// -------------------------------------------------------------------------
	for (int i = 0; i < SMLCListWidget->count(); ++i) {
		delete current_db_clients.value(SMLCListWidget->item(i));
	}
	current_db_clients.clear(); SMLCListWidget->clear(); SMSLListWidget->clear();
	enableAll(); actionQuit->setEnabled(true);
	actionNew->setEnabled(true); actionOpen->setEnabled(true);
	setPage(actionEdit_test); togglePrintEnabled();
	if (actionUse_last_save_date->isChecked()) {
		actionUse_last_save_date->setChecked(false);
		save();
		actionUse_last_save_date->setChecked(true);
	} else { save(); }
}

void MainWindow::addClient()
{
    QTcpSocket * clientConnection = tcpServer->nextPendingConnection();
    Client * client = new Client (this, clientConnection);
    /*QObject::connect(clientConnection, SIGNAL(disconnected()),
                  clientConnection, SLOT(deleteLater()));*/
    QObject::connect(clientConnection, SIGNAL(readyRead()), client, SLOT(readClientFeedback()));
    QObject::connect(clientConnection, SIGNAL(error(QAbstractSocket::SocketError)),
                  client, SLOT(displayError(QAbstractSocket::SocketError)));
    QObject::connect(client, SIGNAL(identified(Client *)),
                  this, SLOT(clientIdentified(Client *)));
    QObject::connect(client, SIGNAL(finished(Client *)),
                  this, SLOT(clientFinished(Client *)));
    QObject::connect(client, SIGNAL(resultsLoaded(Client *)),
                  this, SLOT(clientResultsLoaded(Client *)));
    QObject::connect(client, SIGNAL(disconnected(Client *)),
                  this, SLOT(clientDisconnected(Client *)));
    
    QListWidgetItem * item = new QListWidgetItem (QString("%1").arg(SMLCListWidget->count()+1), SMLCListWidget);
    current_db_clients.insert(item, client);
    client->setNumber(item->text().toInt());
    QListWidgetItem * log_entry = new QListWidgetItem (tr("%1 > Client #%2 connected").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")).arg(item->text()));
    SMSLListWidget->insertItem(0, log_entry);
    log_entry->setBackground(QBrush::QBrush(QColor::QColor(197, 255, 120)));
    log_entry->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
    updateLC(client);
    
    QByteArray client_number;
    QDataStream out1(&client_number, QIODevice::WriteOnly);
    out1.setVersion(QDataStream::Qt_4_2);
    out1 << (quint32)(client->number() + QDate::currentDate().day() + QTime::currentTime().hour());
    
    QByteArray num_entries;
    QDataStream out2(&num_entries, QIODevice::WriteOnly);
    out2.setVersion(QDataStream::Qt_4_2);
    out2 << (quint64)current_db_test.count();

    clientConnection->write(client_number);
    clientConnection->write(num_entries);
    clientConnection->write(current_db_test.at(0));
    //clientConnection->disconnectFromHost();
}

void MainWindow::clientIdentified(Client * client)
{
    QListWidgetItem * log_entry = new QListWidgetItem (tr("%1 > Client #%2 identified as %3").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")).arg(client->number()).arg(client->name()));
    SMSLListWidget->insertItem(0, log_entry);
    log_entry->setBackground(QBrush::QBrush(QColor::QColor(17, 120, 122)));
    log_entry->setForeground(QBrush::QBrush(QColor::QColor(255, 255, 255)));
    if (current_db_clients.value(SMLCListWidget->currentItem()) == client)
    { setCurrentClient(); }
    updateLC(client); toggleSaveSessionEnabled();
}

void MainWindow::clientFinished(Client * client)
{
    QListWidgetItem * log_entry = new QListWidgetItem (tr("%1 > Client #%2 (%3) finished the exam").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")).arg(client->number()).arg(client->name()));
    SMSLListWidget->insertItem(0, log_entry);
    log_entry->setBackground(QBrush::QBrush(QColor::QColor(255, 251, 0)));
    log_entry->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
}

void MainWindow::clientResultsLoaded(Client * client)
{
    QListWidgetItem * log_entry = new QListWidgetItem (tr("%1 > Client #%2 (%3) submitted results").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")).arg(client->number()).arg(client->name()));
    SMSLListWidget->insertItem(0, log_entry);
    log_entry->setBackground(QBrush::QBrush(QColor::QColor(221, 255, 0)));
    log_entry->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
    if (current_db_clients.value(SMLCListWidget->currentItem()) == client)
    { setCurrentClient(); }
    updateLC(client); sendCorrectAnswers(client);
    if (!printClientResults(client, default_printer)) {
        QListWidgetItem * log_entry = new QListWidgetItem (tr("%1 > Client #%2 (%3) > failed to print the client's results").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")).arg(client->number()).arg(client->name()));
        SMSLListWidget->insertItem(0, log_entry);
        log_entry->setBackground(QBrush::QBrush(QColor::QColor(255, 0, 0)));
        log_entry->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
    } else {
        QListWidgetItem * log_entry = new QListWidgetItem (tr("%1 > Client #%2 (%3) > results printed successfully").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")).arg(client->number()).arg(client->name()));
        SMSLListWidget->insertItem(0, log_entry);
        log_entry->setBackground(QBrush::QBrush(QColor::QColor(221, 255, 0)));
        log_entry->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
    }
}

void MainWindow::clientDisconnected(Client * client)
{
    QListWidgetItem * log_entry = new QListWidgetItem;
    SMSLListWidget->insertItem(0, log_entry);
    if (client->isIdentified()) {
       log_entry->setText(tr("%1 > Client #%2 (%3) disconnected").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")).arg(client->number()).arg(client->name()));
    } else {
       log_entry->setText(tr("%1 > Client #%2 disconnected").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")).arg(client->number()));
    }
    log_entry->setBackground(QBrush::QBrush(QColor::QColor(204, 109, 0)));
    log_entry->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
    if (current_db_clients.value(SMLCListWidget->currentItem()) == client)
    { setCurrentClient(); }
    updateLC(client);
}

void MainWindow::setCurrentClient()
{
	if (SMLCListWidget->currentIndex().isValid()) {
		if (SMLCListWidget->currentItem()->isHidden())
		{ disableSMSC(); clearSMSC(); togglePrintEnabled(); return; }
		enableSMSC(); clearSMSC(); togglePrintEnabled();
		Client * client = current_db_clients.value(SMLCListWidget->currentItem());
		if (client->isIdentified()) { SMSCNameLabel->setText(client->name()); }
		else { SMSCNameLabel->setText(tr("Not identified yet")); }
		setClientStatus(client->socket()->state());
		if (client->isReady()) {
			SMSCScoreLabel->setText(tr("%1 out of %2 (%3)").arg(client->score()).arg(TSQnumSpinBox->value()).arg(client->passed() ? tr("PASSED") : tr("FAILED")));
			loadClientResults(client->results(), SMSCResultsTableWidget);
		}
	} else {
		disableSMSC(); clearSMSC(); togglePrintEnabled();
	}
}

void MainWindow::loadClientResults(QMap<QString, QuestionAnswer> * results, QTableWidget * tw)
{
    QMapIterator<QString, QuestionAnswer> i(*results); int row = 0;
    QTableWidgetItem * item; QuestionAnswer qans; QuestionItem * q_item;
    tw->setRowCount(results->count());
    QStringList bufferlist;
    while (i.hasNext()) {
       i.next();
       item = new QTableWidgetItem (i.key()); qans = i.value();
       q_item = NULL;
       QMapIterator<QListWidgetItem *, QuestionItem *> q(current_db_questions);
       while (q.hasNext()) { q.next();
           if (q.value()->name() == i.key()) { q_item = q.value(); break; }
       }
       if (q_item != NULL) {
    	   item->setText(q_item->group().isEmpty() ? q_item->name() : QString("[%1] %2").arg(q_item->group()).arg(q_item->name()));
    	   item->setIcon(iconForDifficulty(q_item->difficulty()));
       }
       item->setBackground(QBrush::QBrush(backgroundColourForFlag(qans.flag())));
       item->setForeground(QBrush::QBrush(foregroundColourForFlag(qans.flag())));
       tw->setItem(row, 0, item);
       item = new QTableWidgetItem (qans.isAnsweredCorrectly() ? tr("yes") : tr("no"));
       if (qans.isAnsweredCorrectly()) {
          item->setBackground(QBrush::QBrush(QColor::QColor(197, 255, 120)));
          item->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
       } else {
          item->setBackground(QBrush::QBrush(QColor::QColor(204, 109, 0)));
          item->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
       }
       tw->setItem(row, 1, item);
       item = new QTableWidgetItem;
       switch (qans.answered()) {
          case QuestionItem::None: item->setText(tr("None")); break;
          case QuestionItem::A: item->setText(tr("a)")); break;
          case QuestionItem::B: item->setText(tr("b)")); break;
          case QuestionItem::C: item->setText(tr("c)")); break;
          case QuestionItem::D: item->setText(tr("d)")); break;
          default: item->setText(tr("None")); break;
       }
       tw->setItem(row, 2, item);
       item = new QTableWidgetItem;
       bufferlist.clear();
       if ((QuestionItem::A & qans.correctAnswer()) == QuestionItem::A) bufferlist << tr("a)");
       if ((QuestionItem::B & qans.correctAnswer()) == QuestionItem::B) bufferlist << tr("b)");
       if ((QuestionItem::C & qans.correctAnswer()) == QuestionItem::C) bufferlist << tr("c)");
       if ((QuestionItem::D & qans.correctAnswer()) == QuestionItem::D) bufferlist << tr("d)");
       if (bufferlist.count() > 0) { item->setText(bufferlist.join(", ")); }
           else { item->setText(tr("None")); }
       tw->setItem(row, 3, item);
       row++;
    }
    tw->resizeRowsToContents();
}

void MainWindow::sendCorrectAnswers(Client * client)
{
    QMap<QString, QuestionAnswer> * results = client->results();
    QByteArray correct_answers;
    QDataStream out(&correct_answers, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << (quint64)0;
    // -------------------------------------------------------------------------
    QMapIterator<QString, QuestionAnswer> i(*results); QuestionAnswer qans;
    while (i.hasNext()) {
        i.next(); qans = i.value();
        out << QString("[Q_NAME]\n") << i.key() << QString("\n[Q_C_ANS]\n");
        out << QString("%1\n").arg(qans.correctAnswer());
    }
    // -------------------------------------------------------------------------
    out.device()->seek(0);
    out << (quint64)(correct_answers.size() - sizeof(quint64));
    client->socket()->write(correct_answers);
}

void MainWindow::updateLC(Client * client)
{
    QList<QListWidgetItem *> client_items = current_db_clients.keys(client);
    for (int i = 0; i < client_items.count(); ++i) {
       if ((!client->isIdentified()) &&
          (client->socket()->state() == QAbstractSocket::UnconnectedState)) {
             client_items.at(i)->setHidden(true);
       } else {client_items.at(i)->setHidden(false);}
       if (client->isIdentified())
          client_items.at(i)->setText(client->name());
       switch (client->socket()->state()) {
          case QAbstractSocket::UnconnectedState:
             client_items.at(i)->setBackground(QBrush::QBrush(QColor::QColor(204, 109, 0)));
             client_items.at(i)->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
             break;
          case QAbstractSocket::ConnectedState:
             client_items.at(i)->setBackground(QBrush::QBrush(QColor::QColor(197, 255, 120)));
             client_items.at(i)->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
             break;
          case QAbstractSocket::HostLookupState:
          case QAbstractSocket::ConnectingState:
          case QAbstractSocket::BoundState:
          case QAbstractSocket::ClosingState:
          case QAbstractSocket::ListeningState:
          default:
             client_items.at(i)->setBackground(QBrush::QBrush(QColor::QColor(201, 204, 0)));
             client_items.at(i)->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
             break;
       }
    }
}

int MainWindow::numIdentifiedClients()
{
    int num = 0;
    QMapIterator<QListWidgetItem *, Client *> i(current_db_clients);
	while (i.hasNext()) { i.next(); if (i.value()->isIdentified()) { num++; } }
    return num;
}

void MainWindow::toggleSaveSessionEnabled()
{
    if ((mainStackedWidget->currentIndex() == 5) && (numIdentifiedClients() > 0)) {
        actionSave_session->setEnabled(true);
    } else { actionSave_session->setEnabled(false); }
}

void MainWindow::setClientStatus(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
       case QAbstractSocket::UnconnectedState:
          SMSCStatusLabel->setText(tr("Disconnected")); break;
       case QAbstractSocket::HostLookupState:
          SMSCStatusLabel->setText(tr("Performing a host name lookup")); break;
       case QAbstractSocket::ConnectingState:
          SMSCStatusLabel->setText(tr("Connecting")); break;
       case QAbstractSocket::ConnectedState:
          SMSCStatusLabel->setText(tr("Connected")); break;
       case QAbstractSocket::BoundState:
          SMSCStatusLabel->setText(tr("Bound to an address and port")); break;
       case QAbstractSocket::ClosingState:
          SMSCStatusLabel->setText(tr("Closing")); break;
       case QAbstractSocket::ListeningState:
          SMSCStatusLabel->setText(tr("Listening")); break;
       default:
          SMSCStatusLabel->setText(tr("Unknown")); break;
    }
}

void MainWindow::addOfflineClient()
{
    QString file_name = QFileDialog::getOpenFileName(this, tr("Add off-line client"), "", tr("iTest off-line client logs (*.itcl);;All files (*.*)"));
    if (file_name.isEmpty()) { return; }
    if (!addOfflineClient(file_name)) {
        QMessageBox::critical(this, tr("Add off-line client"), tr("Failed to add off-line client. Either this file contains client data of a different test session or it contains invalid data."));
    }
}

bool MainWindow::addOfflineClient(QString file_name)
{
    if (file_name.isEmpty()) return false;
    QFile file(file_name);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(this, tr("Add off-line client"), tr("Cannot read file %1:\n%2.").arg(file_name).arg(file.errorString()));
        return true;
    }
    QTextStream rfile(&file);
    rfile.setCodec("UTF-8");
    
    if (rfile.readLine() != "[TEST_NAME]") return false;
    if (rfile.readLine() != current_db_testname) return false;
    if (rfile.readLine() != "[TEST_TIMESTAMP]") return false;
    if (rfile.readLine() != current_db_testdate) return false;
    if (rfile.readLine() != "[TIME_FINISHED]") return false;
    rfile.readLine();
    if (rfile.readLine() != "[CLIENT_NAME]") return false;
    Client * client = new Client (this, rfile.readLine());
    QListWidgetItem * item = new QListWidgetItem (QString("%1").arg(SMLCListWidget->count()+1), SMLCListWidget);
    current_db_clients.insert(item, client);
    client->setNumber(item->text().toInt()); item->setText(client->name());
    QListWidgetItem * log_entry = new QListWidgetItem (tr("%1 > Client #%2 (%3) added").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")).arg(client->number()).arg(client->name()));
    SMSLListWidget->insertItem(0, log_entry);
    log_entry->setBackground(QBrush::QBrush(QColor::QColor(0, 125, 163)));
    log_entry->setForeground(QBrush::QBrush(QColor::QColor(255, 255, 255)));
    updateLC(client); toggleSaveSessionEnabled();
    QObject::connect(client, SIGNAL(resultsLoaded(Client *)),
                  this, SLOT(clientResultsLoaded(Client *)));
    client->loadResults(rfile.readAll());
    
    return true;
}

void MainWindow::addOfflineClients()
{
#ifdef Q_WS_WIN
    QString dir_path = QFileDialog::getExistingDirectory(this, tr("Add off-line clients"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
#else
    QString dir_path = QFileDialog::getExistingDirectory(this, tr("Add off-line clients"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
#endif
    if (dir_path.isEmpty()) { return; }
    QDir dir (dir_path);
    QStringList filter; filter << "*.itcl";
    QFileInfoList client_logs = dir.entryInfoList(filter, QDir::Files, QDir::Time);
    if (client_logs.count() == 0) { QMessageBox::information(this, tr("Add off-line clients"), tr("No iTest client logs found in the folder you selected.")); }
    for (int i = 0; i < client_logs.count(); ++i) {
        if (!addOfflineClient(client_logs.at(i).absoluteFilePath())) {
            QListWidgetItem * log_entry = new QListWidgetItem (tr("%1 > Failed to add off-line client").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss")));
            SMSLListWidget->insertItem(0, log_entry);
            log_entry->setBackground(QBrush::QBrush(QColor::QColor(204, 109, 0)));
            log_entry->setForeground(QBrush::QBrush(QColor::QColor(0, 0, 0)));
        }
    }
}

void MainWindow::exportTest()
{
	QString datetime = current_db_testdate; datetime.replace(13, 1, '.');
    QString save_file_name = QFileDialog::getSaveFileName(this, tr("Export test"), QString("%1-%2.itos").arg(current_db_testname).arg(datetime), tr("iTest off-line test sessions (*.itos)"));
	if (save_file_name.isNull() || save_file_name.isEmpty()) { return; }
	QFile file(save_file_name);
	if (!file.open(QFile::WriteOnly | QFile::Text)) 
	{
		QMessageBox::critical(this, tr("Export test"), tr("Cannot write file %1:\n%2.").arg(save_file_name).arg(file.errorString()));
		return;
	}
	QTextStream sfile(&file);
	sfile.setCodec("UTF-8");
	for (int i = 0; i < current_db_test.count(); ++i) {
        QDataStream in(current_db_test.at(i));
        in.setVersion(QDataStream::Qt_4_2);
        quint64 num; in >> num; QString buffer;
        do {
            in >> buffer; sfile << buffer;
        } while (!in.atEnd());
        setProgress(100 / current_db_test.count() * (i + 1)); // PROGRESS >>>>>>
    }
    setProgress(100); setProgress(-1); // PROGRESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	statusBar()->showMessage(tr("Test exported"), 10000);
}

void MainWindow::exportLog()
{
/* LOG FORMAT
<html><head><meta http-equiv="Content-Type" content="text/html; charset=utf-8"><title>
iTest Server Log - <db_name> - <timestamp>
</title></head><body>
<div style="background-color: rgb(197, 255, 120); color: rgb(0, 0, 0)">
Text
</div>
</body></html>
*/
    QListWidget * lw; QString testname; QString datetime;
    if (mainStackedWidget->currentIndex() == 5) {
        lw = SMSLListWidget; testname = current_db_testname;
        datetime = QDateTime::currentDateTime().toString("yyyy.MM.dd-hh.mm");
    } else if (mainStackedWidget->currentIndex() == 6) {
        if (current_db_session == NULL) { return; }
        lw = VSSSLListWidget; testname = current_db_session->name();
        datetime = current_db_session->dateTimeToString();
        datetime.replace(13, 1, '.');
    } else { return; }
	QString timestamp = datetime; timestamp.replace(13, 1, ':');
	QString save_file_name = QFileDialog::getSaveFileName(this, tr("Export server log"), QString("%1-%2.html").arg(testname).arg(datetime), tr("iTest Server logs (*.html)"));
	if (save_file_name.isNull() || save_file_name.isEmpty()) { return; }
	QFile file(save_file_name);
	if (!file.open(QFile::WriteOnly | QFile::Text)) 
	{
		QMessageBox::critical(this, tr("Export server log"), tr("Cannot write file %1:\n%2.").arg(save_file_name).arg(file.errorString()));
		return;
	}
	QTextStream sfile(&file);
	sfile.setCodec("UTF-8");
	setProgress(0); // PROGRESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	sfile << "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><title>" << endl;
	sfile << tr("iTest Server Log") << " - " << testname << " - ";
	sfile << timestamp << endl << "</title></head><body>" << endl;
	for (int i = 0; i < lw->count(); ++i) {
		sfile << "<div style=\"background-color: rgb(";
		sfile << lw->item(i)->background().color().red() << ", ";
		sfile << lw->item(i)->background().color().green() << ", ";
		sfile << lw->item(i)->background().color().blue() << "); ";
		sfile << "color: rgb(";
		sfile << lw->item(i)->foreground().color().red() << ", ";
		sfile << lw->item(i)->foreground().color().green() << ", ";
		sfile << lw->item(i)->foreground().color().blue() << ")\">";
		sfile << endl << lw->item(i)->text() << "\n</div>" << endl;
		setProgress(100 / lw->count() * i); // PROGRESS >>>>>>>>>>>>>>>>>>>>>>>>
	}
	sfile << "</body></html>" << endl;
	setProgress(100); setProgress(-1); // PROGRESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// -------------------------------------------------------------------------
}

void MainWindow::runTestWriter()
{
	QFileInfo file_info(qApp->arguments().at(0));
	QString itw_file_name = file_info.absoluteFilePath();
#ifdef Q_WS_MAC
	itw_file_name.replace(itw_file_name.lastIndexOf("iTestServer.app"), 15, "iTestClient.app");
	itw_file_name.replace(itw_file_name.lastIndexOf("Server"), 6, "Client");
#else
	itw_file_name.replace(itw_file_name.lastIndexOf("iTestServer"), 11, "iTestClient");
#endif
	QStringList arguments; arguments << "-port" << QString("%1").arg(tcpServer->serverPort());
	QProcess * itw = new QProcess;
	itw->start(itw_file_name, arguments);
}

void MainWindow::enableSMSC()
{
/*  SMSCNameLabel->setEnabled(true);
    SMSCStatusLabel->setEnabled(true);
    SMSCScoreLabel->setEnabled(true);
    SMSCResultsTableWidget->setEnabled(true);*/
    
    SMSelectedClientGroupBox->setEnabled(true);
}

void MainWindow::disableSMSC()
{
/*  SMSCNameLabel->setEnabled(false);
    SMSCStatusLabel->setEnabled(false);
    SMSCScoreLabel->setEnabled(false);
    SMSCResultsTableWidget->setEnabled(false);*/
    
    SMSelectedClientGroupBox->setEnabled(false);
}

void MainWindow::clearSMSC()
{
    SMSCNameLabel->setText(tr("Not identified yet"));
    SMSCStatusLabel->setText(tr("Disconnected"));
    SMSCScoreLabel->setText(tr("Not ready yet"));
    SMSCResultsTableWidget->setRowCount(0);
}

void MainWindow::enableSMTools()
{
    switch (mainStackedWidget->currentIndex()) {
		case 4:
    		menuServer->setEnabled(true);
    		actionStart_server->setEnabled(true);
    		actionStop_server->setEnabled(false);
    		actionRun_Test_Writer->setEnabled(false);
    		actionExport_test->setEnabled(false);
    		actionAdd_offline_client->setEnabled(false);
    		actionAdd_offline_clients->setEnabled(false);
    		actionSave_session->setEnabled(false);
    		break;
    	case 5:
    		menuServer->setEnabled(true);
    		actionStart_server->setEnabled(false);
    		actionStop_server->setEnabled(true);
    		actionRun_Test_Writer->setEnabled(true);
    		actionExport_test->setEnabled(true);
    		actionAdd_offline_client->setEnabled(true);
    		actionAdd_offline_clients->setEnabled(true);
    		toggleSaveSessionEnabled();
    		break;
    	default:
    		menuServer->setEnabled(false);
    		actionStart_server->setEnabled(false);
    		actionStop_server->setEnabled(false);
    		actionRun_Test_Writer->setEnabled(false);
    		actionExport_test->setEnabled(false);
    		actionAdd_offline_client->setEnabled(false);
    		actionAdd_offline_clients->setEnabled(false);
    		actionSave_session->setEnabled(false);
    		break;
    }
}

void MainWindow::togglePrintEnabled()
{
    if (mainStackedWidget->currentIndex() >= 1 && mainStackedWidget->currentIndex() <= 4) {
        actionPrint_questions->setEnabled(true);
        actionQuickPrint->setEnabled(false);
        actionPrint->setEnabled(false);
        actionPrint_session_summary->setEnabled(false);
        actionPrint_all->setEnabled(false);
    } else if ((mainStackedWidget->currentIndex() == 5) && (SMLCListWidget->currentIndex().isValid())) {
        actionPrint_questions->setEnabled(false);
        actionQuickPrint->setEnabled(true);
        actionPrint->setEnabled(true);
        actionPrint_session_summary->setEnabled(false);
        actionPrint_all->setEnabled(false);
    } else if ((mainStackedWidget->currentIndex() == 6) && (VSSLCListWidget->currentIndex().isValid())) {
        actionPrint_questions->setEnabled(true);
        actionQuickPrint->setEnabled(false);
        actionPrint->setEnabled(true);
        if (VSSCSGroupBox->isEnabled()) {
            actionPrint_session_summary->setEnabled(true);
        } else { actionPrint_session_summary->setEnabled(false); }
        if (VSSLCListWidget->count() > 0) { actionPrint_all->setEnabled(true); }
        else { actionPrint_all->setEnabled(false); }
    } else if ((mainStackedWidget->currentIndex() == 6) && (!VSSLCListWidget->currentIndex().isValid())) {
        actionPrint_questions->setEnabled(true);
        actionQuickPrint->setEnabled(false);
        actionPrint->setEnabled(false);
        if (VSSCSGroupBox->isEnabled()) {
            actionPrint_session_summary->setEnabled(true);
        } else { actionPrint_session_summary->setEnabled(false); }
        if (VSSLCListWidget->count() > 0) { actionPrint_all->setEnabled(true); }
        else { actionPrint_all->setEnabled(false); }
    } else {
        actionPrint_questions->setEnabled(false);
        actionQuickPrint->setEnabled(false);
        actionPrint->setEnabled(false);
        actionPrint_session_summary->setEnabled(false);
        actionPrint_all->setEnabled(false);
    }
}

void MainWindow::clearSM()
{
	TSAdvancedSetupGroupBox->setChecked(false);
	rbtnSelectFlags->setChecked(true);
	TSSearchAvailableLineEdit->clear();
	TSSearchUsedLineEdit->clear();
	TSExcludeListWidget->clear();
	TSIncludeTableWidget->clear();
	TSIncludeTableWidget->setRowCount(0);
	TSGroupsCheckBox->setChecked(false);
	TSMaxQnumCheckBox->setChecked(false);
	rbtnTestTime->setChecked(true);
	TSTestTimeEdit->setTime(QTime::QTime(0, 0));
	TSQuestionTimeEdit->setTime(QTime::QTime(0, 0));
}