#include <QtSql>
#include <QMessageBox>
#include <QDate>

#include "wid_osnovni_pogled.h"
#include "ui_wid_osnovni_pogled.h"
#include "kodiranje.h"
#include "varnost.h"
#include "datum.h"
#include "projekti.h"
#include "potninalogi.h"
#include "prejetiracuni.h"
#include "racun.h"

wid_osnovni_pogled::wid_osnovni_pogled(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::wid_osnovni_pogled)
{
	ui->setupUi(this);


	napolni_projekte();
	napolni_stranke();
	napolni_potne_naloge();
	napolni_prejete_racune();
	napolni_izdane_racune();
	napolni_predracune();

}

wid_osnovni_pogled::~wid_osnovni_pogled()
{
	delete ui;
}

QString wid_osnovni_pogled::pretvori(QString besedilo) {

	return kodiranje().zakodiraj(besedilo);

}

QString wid_osnovni_pogled::prevedi(QString besedilo) {

	return kodiranje().odkodiraj(besedilo);

}

void wid_osnovni_pogled::napolni_projekte() {

		// prestej odprte projekte
		int odprti_projekti = 0;
		QStringList seznam_projektov;

		QString datum = "." + QDate::currentDate().toString("MM") + "." + QDate::currentDate().toString("yyyy");

		QSqlQuery sql_odprti_projekti;
		// stevilo projektov, ki so trenutno v teku
		sql_odprti_projekti.prepare("SELECT * FROM projekti WHERE status_projekta LIKE '" + pretvori("2") + "'"); // projekt je v teku
		//        Za primer, ko gledamo samo stevilo projektov, ki so bili potrjeni v tekocem mesecu
		//        sql_odprti_projekti.prepare("SELECT * FROM racuni WHERE tip_racuna LIKE '" + pretvori("1") +
		//                                    "' AND status_racuna LIKE 'Pmoilii' AND datum_oddaje_racuna LIKE '%" + datum + "%'");
		sql_odprti_projekti.exec();
		while ( sql_odprti_projekti.next() ) {
			// stevilo projektov, ki so trenutno v teku
			odprti_projekti++;
			//            Za primer, ko gledamo samo stevilo projektov, ki so bili potrjeni v tekocem mesecu
			//            if ( !seznam_projektov.contains(prevedi(sql_odprti_projekti.value(sql_odprti_projekti.record().indexOf("projekt")).toString())) ) {
			//                seznam_projektov.append(prevedi(sql_odprti_projekti.value(sql_odprti_projekti.record().indexOf("projekt")).toString()));
			//                odprti_projekti++;
			//            }
		}
		seznam_projektov.clear();

		ui->txt_odprti_projekti->setText(QString::number(odprti_projekti, 10));

		// prestej zavrnjene predracune / zavrnjene projekte
		int zavrnjeni_projekti = 0;

		QSqlQuery sql_zavrnjeni_projekti;
		sql_zavrnjeni_projekti.prepare("SELECT * FROM racuni WHERE tip_racuna LIKE '" + pretvori("1") +
									   "' AND status_racuna LIKE 'Zvripnzj' AND datum_oddaje_racuna LIKE '%" + datum + "%'");
		sql_zavrnjeni_projekti.exec();
		while ( sql_zavrnjeni_projekti.next() ) {
			if ( !seznam_projektov.contains(prevedi(sql_zavrnjeni_projekti.value(sql_zavrnjeni_projekti.record().indexOf("projekt")).toString())) ) {
				seznam_projektov.append(prevedi(sql_zavrnjeni_projekti.value(sql_zavrnjeni_projekti.record().indexOf("projekt")).toString()));
				zavrnjeni_projekti++;
			}
		}

		ui->txt_zavrnjeni_projekti->setText(QString::number(zavrnjeni_projekti, 10));

}


void wid_osnovni_pogled::on_btn_prioriteta_gor_clicked() {

	QString gumb = ui->btn_prioriteta_gor->text();
	ui->btn_prioriteta_gor->setText("Premikam");
	ui->btn_prioriteta_gor->setEnabled(false);
	ui->btn_prioriteta_dol->setEnabled(false);
	qApp->processEvents();

	if ( ui->tbl_stranke->selectedItems().count() > 0 ) { // preveri, ce sploh imamo izbrano vrstico

		// zapomni si trenutno prioriteto, ce je NULL jo spremeni v zadnjo
		if ( ui->tbl_stranke->selectedItems().at(5)->text() == "NULL" ) {
			QTableWidgetItem *vnos_prioritete = new QTableWidgetItem;
			vnos_prioritete->setText(QString::number(ui->tbl_stranke->selectedItems().at(0)->row() + 1, 10));
			ui->tbl_stranke->setItem(ui->tbl_stranke->selectedItems().at(0)->row(), 5, vnos_prioritete);
		}
		int osnovna = ui->tbl_stranke->selectedItems().at(5)->text().toInt();

		if ( osnovna > 1 ) { // lahko jo prestavimo navzgor, prioriteta 1 je namrec najvisja in z njo ne moremo narediti kaj dosti
			// zamenjaj vrednosti prioritete z naslednjo na seznamu (eno vrstico visje)

			QString id_zacetna = ui->tbl_stranke->selectedItems().at(0)->text();
			QString ime_zacetna = ui->tbl_stranke->selectedItems().at(1)->text();
			QString kontakt_zacetna = ui->tbl_stranke->selectedItems().at(2)->text();
			QString projekt_zacetna = ui->tbl_stranke->selectedItems().at(3)->text();
			QString datum_zacetna = ui->tbl_stranke->selectedItems().at(4)->text();
			QString prioriteta_zacetna = QString::number(osnovna - 1, 10);

			ui->tbl_stranke->selectRow(ui->tbl_stranke->selectedItems().at(0)->row() - 1);

			QString id_koncna = ui->tbl_stranke->selectedItems().at(0)->text();
			QString ime_koncna = ui->tbl_stranke->selectedItems().at(1)->text();
			QString kontakt_koncna = ui->tbl_stranke->selectedItems().at(2)->text();
			QString projekt_koncna = ui->tbl_stranke->selectedItems().at(3)->text();
			QString datum_koncna = ui->tbl_stranke->selectedItems().at(4)->text();
			QString prioriteta_koncna = QString::number(osnovna, 10);

			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() + 1, 0)->setText(id_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() + 1, 1)->setText(ime_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() + 1, 2)->setText(kontakt_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() + 1, 3)->setText(projekt_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() + 1, 4)->setText(datum_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() + 1, 5)->setText(prioriteta_koncna);

			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 0)->setText(id_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 1)->setText(ime_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 2)->setText(kontakt_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 3)->setText(projekt_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 4)->setText(datum_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 5)->setText(prioriteta_zacetna);
		}

		shrani_prioriteto();
	}

	ui->btn_prioriteta_gor->setText(gumb);
	ui->btn_prioriteta_gor->setEnabled(true);
	ui->btn_prioriteta_dol->setEnabled(true);
	qApp->processEvents();

}

void wid_osnovni_pogled::on_btn_prioriteta_dol_clicked() {

	QString gumb = ui->btn_prioriteta_dol->text();
	ui->btn_prioriteta_dol->setText("Premikam");
	ui->btn_prioriteta_gor->setEnabled(false);
	ui->btn_prioriteta_dol->setEnabled(false);
	qApp->processEvents();

	if ( ui->tbl_stranke->selectedItems().count() > 0 ) { // preveri, ce sploh imamo izbrano vrstico

		// zapomni si trenutno prioriteto, ce je NULL jo spremeni v zadnjo
		if ( ui->tbl_stranke->selectedItems().at(5)->text() == "NULL" ) {
			QTableWidgetItem *vnos_prioritete = new QTableWidgetItem;
			vnos_prioritete->setText(QString::number(ui->tbl_stranke->selectedItems().at(0)->row() + 1, 10));
			ui->tbl_stranke->setItem(ui->tbl_stranke->selectedItems().at(0)->row(), 5, vnos_prioritete);
		}
		int osnovna = ui->tbl_stranke->selectedItems().at(5)->text().toInt();

		if ( osnovna < ui->tbl_stranke->rowCount() ) { // lahko jo prestavimo navzdol, ne moremo iti pa nizje kot je stevilo vrstic v tabeli
			// zamenjaj vrednosti prioritete z naslednjo na seznamu (eno vrstico nizje)

			QString id_zacetna = ui->tbl_stranke->selectedItems().at(0)->text();
			QString ime_zacetna = ui->tbl_stranke->selectedItems().at(1)->text();
			QString kontakt_zacetna = ui->tbl_stranke->selectedItems().at(2)->text();
			QString projekt_zacetna = ui->tbl_stranke->selectedItems().at(3)->text();
			QString datum_zacetna = ui->tbl_stranke->selectedItems().at(4)->text();
			QString prioriteta_zacetna = QString::number(osnovna + 1, 10);

			ui->tbl_stranke->selectRow(ui->tbl_stranke->selectedItems().at(0)->row() + 1);

			QString id_koncna = ui->tbl_stranke->selectedItems().at(0)->text();
			QString ime_koncna = ui->tbl_stranke->selectedItems().at(1)->text();
			QString kontakt_koncna = ui->tbl_stranke->selectedItems().at(2)->text();
			QString projekt_koncna = ui->tbl_stranke->selectedItems().at(3)->text();
			QString datum_koncna = ui->tbl_stranke->selectedItems().at(4)->text();
			QString prioriteta_koncna = QString::number(osnovna, 10);

			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() - 1, 0)->setText(id_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() - 1, 1)->setText(ime_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() - 1, 2)->setText(kontakt_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() - 1, 3)->setText(projekt_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() - 1, 4)->setText(datum_koncna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row() - 1, 5)->setText(prioriteta_koncna);

			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 0)->setText(id_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 1)->setText(ime_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 2)->setText(kontakt_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 3)->setText(projekt_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 4)->setText(datum_zacetna);
			ui->tbl_stranke->item(ui->tbl_stranke->selectedItems().at(0)->row(), 5)->setText(prioriteta_zacetna);
		}

		shrani_prioriteto();

	}

	ui->btn_prioriteta_dol->setText(gumb);
	ui->btn_prioriteta_gor->setEnabled(true);
	ui->btn_prioriteta_dol->setEnabled(true);
	qApp->processEvents();

}


void wid_osnovni_pogled::on_btn_osvezi_stranke_clicked() {

	napolni_stranke();

}

void wid_osnovni_pogled::napolni_stranke() {

	int izbranec = 0;
	int razvrsti = 5;

	if ( ui->tbl_stranke->selectedItems().count() > 0 ) {
		izbranec = ui->tbl_stranke->selectedItems().takeAt(0)->row();
	}

//    razvrsti = ui->tbl_stranke->horizontalHeader()->sortIndicatorSection();

	ui->tbl_stranke->clear();

	for (int i = 0; i <= 6; i++) {
		ui->tbl_stranke->removeColumn(0);
	}

	QSqlQuery sql_clear;
	sql_clear.prepare("SELECT * FROM stranke");
	sql_clear.exec();
	while (sql_clear.next()) {
		ui->tbl_stranke->removeRow(0);
	}

	// start filling the table
	ui->tbl_stranke->insertColumn(0);
	ui->tbl_stranke->insertColumn(1);
	ui->tbl_stranke->insertColumn(2);
	ui->tbl_stranke->insertColumn(3);
	ui->tbl_stranke->insertColumn(4);
	ui->tbl_stranke->insertColumn(5);

	// set proper width to the first four columns
	ui->tbl_stranke->setColumnWidth(0, 0);
	ui->tbl_stranke->setColumnWidth(1, 150);
	ui->tbl_stranke->setColumnWidth(2, 150);
	ui->tbl_stranke->setColumnWidth(3, 100);
	ui->tbl_stranke->setColumnWidth(4, 100);
	ui->tbl_stranke->setColumnWidth(5, 0);

	// start filling the table
	QStringList naslovi;
	naslovi.append("ID");
	naslovi.append("Ime / Naziv");
	naslovi.append("Kontakt");
	naslovi.append("Projekt");
	naslovi.append("Datum");
	naslovi.append("Prioriteta");

	ui->tbl_stranke->setHorizontalHeaderLabels(naslovi);

	// select open projects (status '2')
	int row = 0;

	QSqlQuery sql_projekti;
	sql_projekti.prepare("SELECT * FROM projekti WHERE status_projekta LIKE '" + pretvori("2") + "'"); // projekt je v teku
	sql_projekti.exec();
	while ( sql_projekti.next() ) {

		QString stranka = "";
		QString telefon = "";
		QString project = "";
		QString end_date = "";

		// get custumers id from open projects
		QSqlQuery sql_custumer;
		sql_custumer.prepare("SELECT * FROM stranke WHERE id LIKE '" + sql_projekti.value(sql_projekti.record().indexOf("stranka")).toString() + "'");
		sql_custumer.exec();
		if ( sql_custumer.next() ) {
			// get custumers name, surname or bussiness name from that id
			if ( prevedi(sql_custumer.value(sql_custumer.record().indexOf("tip")).toString()) == "1" ) { // fizicna
				stranka = prevedi(sql_custumer.value(sql_custumer.record().indexOf("priimek")).toString()) + " " +
						  prevedi(sql_custumer.value(sql_custumer.record().indexOf("ime")).toString());
			}
			else { // pravna
				stranka = prevedi(sql_custumer.value(sql_custumer.record().indexOf("ime")).toString());
			}

			// get custumers phone number
			if ( prevedi(sql_custumer.value(sql_custumer.record().indexOf("gsm")).toString()) != "+(0)/--" ) {
				telefon = prevedi(sql_custumer.value(sql_custumer.record().indexOf("gsm")).toString());
			}
			else if ( prevedi(sql_custumer.value(sql_custumer.record().indexOf("telefon")).toString()) != "+(0)/--" ) {
				telefon = prevedi(sql_custumer.value(sql_custumer.record().indexOf("telefon")).toString());
			}
			else {
				telefon = prevedi(sql_custumer.value(sql_custumer.record().indexOf("email")).toString());
			}

			// get project number
			project = prevedi(sql_projekti.value(sql_projekti.record().indexOf("stevilka_projekta")).toString());

			// get bill id from open projects
			QDate old_date;
			old_date = QDate::fromString("1.1.2000", "d.M.yyyy");
			QSqlQuery sql_bill;
			sql_bill.prepare("SELECT * FROM racuni WHERE projekt LIKE '" + sql_projekti.value(sql_projekti.record().indexOf("id")).toString() +
							 "' AND tip_racuna LIKE '3'");
			sql_bill.exec();
			while ( sql_bill.next() ) {
				QDate new_date;
				new_date = QDate::fromString(prevedi(sql_bill.value(sql_bill.record().indexOf("datum_konca")).toString()), "dd.MM.yyyy");
				// compare which date from bills of an open project and display the last one
				if ( new_date > old_date ) {
					old_date = new_date;
				}
			}
			end_date = old_date.toString("dd.MM.yyyy");
		}

		// insert new row
		ui->tbl_stranke->insertRow(row);
		ui->tbl_stranke->setRowHeight(row, 20);

		int col = 0;
		int i = 0;

		QString polja[5] = {"id", "ime", "telefon", "projekt", "zakljucek"};

		// fill columns
		while ( col < 5 ) {

			QTableWidgetItem *celica = new QTableWidgetItem;
			if ( polja[i] == "id" ) {
				celica->setData(Qt::DisplayRole, prevedi(sql_projekti.value(sql_projekti.record().indexOf(polja[i])).toString()).toInt());
			}
			else if ( polja[i] == "ime" ) {
				celica->setText(stranka);
			}
			else if ( polja[i] == "telefon" ) {
				celica->setText(telefon);
			}
			else if ( polja[i] == "projekt" ) {
				celica->setText(project);
			}
			else if ( polja[i] == "zakljucek" ) {
				celica->setText(end_date);
			}
			else {
				celica->setText("");
			}

			if ( celica->text() != "" ) {
				ui->tbl_stranke->setItem(row, col, celica);
			}

			col++;
			i++;

		}

		QSqlQuery sql_prioriteta;
		sql_prioriteta.prepare("SELECT * FROM stranke_prioriteta WHERE id_stranke LIKE '" + pretvori(ui->tbl_stranke->item(row, 0)->text()) + "'");
		sql_prioriteta.exec();
		if ( sql_prioriteta.next() ) {
			QTableWidgetItem *vnos_prioritete = new QTableWidgetItem;
			vnos_prioritete->setText(prevedi(sql_prioriteta.value(sql_prioriteta.record().indexOf("prioriteta")).toString()));
			ui->tbl_stranke->setItem(row, 5, vnos_prioritete);
		}
		else {
			QTableWidgetItem *vnos_prioritete = new QTableWidgetItem;
			vnos_prioritete->setText("NULL");
			ui->tbl_stranke->setItem(row, 5, vnos_prioritete);
		}

		row++;
	}
	sql_projekti.clear();

	ui->tbl_stranke->selectRow(izbranec);
	ui->tbl_stranke->sortByColumn(razvrsti, Qt::AscendingOrder);

	if ( ui->btn_prioriteta_gor->text() == "Gor" && ui->btn_prioriteta_dol->text() == "Dol" ) { // zagon
		shrani_prioriteto();
	}

}

void wid_osnovni_pogled::shrani_prioriteto() {

	int st_strank = ui->tbl_stranke->rowCount();

	// izbrisemo celotno tabelo, da ne motijo prejsnji vnosi
	QSqlQuery sql_zbrisi;
	sql_zbrisi.prepare("DELETE FROM stranke_prioriteta");
	sql_zbrisi.exec();

	// zanka skozi vse vrstice, shranimo id stranke in pripadajoco prioriteto
	for ( int i = 0; i < st_strank; i++ ) {
		QSqlQuery sql_vnesi;
		sql_vnesi.prepare("INSERT INTO stranke_prioriteta (id_stranke, prioriteta) VALUES (?, ?)");
		sql_vnesi.bindValue(0, pretvori(ui->tbl_stranke->item(i, 0)->text()));
		sql_vnesi.bindValue(1, pretvori(ui->tbl_stranke->item(i, 5)->text()));
		sql_vnesi.exec();
	}

}

void wid_osnovni_pogled::napolni_potne_naloge() {

	int izbranec = 0;
	int razvrsti = 0;

	if ( ui->tbl_potni_nalogi->selectedItems().count() > 0 ) {
		izbranec = ui->tbl_potni_nalogi->selectedItems().takeAt(0)->row();
	}

	razvrsti = ui->tbl_potni_nalogi->horizontalHeader()->sortIndicatorSection();

		ui->tbl_potni_nalogi->clear();

		for (int i = 0; i <= 3; i++) {
			ui->tbl_potni_nalogi->removeColumn(0);
		}

		QSqlQuery sql_clear;
		sql_clear.prepare("SELECT * FROM potni_nalogi");
		sql_clear.exec();
		while (sql_clear.next()) {
			ui->tbl_potni_nalogi->removeRow(0);
		}

		// start filling the table
		ui->tbl_potni_nalogi->insertColumn(0);
		ui->tbl_potni_nalogi->insertColumn(1);
		ui->tbl_potni_nalogi->insertColumn(2);

		// set proper width to the first four columns

		ui->tbl_potni_nalogi->setColumnWidth(0, 0);
		ui->tbl_potni_nalogi->setColumnWidth(1, 100);
		ui->tbl_potni_nalogi->setColumnWidth(2, 150);

		// start filling the table
		QStringList naslovi;
		naslovi.append("ID");
		naslovi.append("Datum");
		naslovi.append("St. naloga");

		ui->tbl_potni_nalogi->setHorizontalHeaderLabels(naslovi);

		datum *delegate = new datum(this);
		ui->tbl_potni_nalogi->setItemDelegateForColumn(1, delegate);

		int st_nalogov = 0;
		int row = 0;

		QString datum = "." + QDate::currentDate().toString("MM") + "." + QDate::currentDate().toString("yyyy");

		QSqlQuery sql_potni_nalog;
		sql_potni_nalog.prepare("SELECT * FROM potni_nalogi WHERE datum_naloga LIKE '%" + datum + "%'");
		sql_potni_nalog.exec();
		while ( sql_potni_nalog.next() ) {
			st_nalogov++;

			// dodaj novo vrstico v tabelo
			ui->tbl_potni_nalogi->insertRow(row);
			ui->tbl_potni_nalogi->setRowHeight(row, 20);

			int col = 0;
			int i = 0;

			QString polja[3] = {"id", "datum_naloga", "stevilka_naloga"};

			// napolni stolpce
			while ( col < 3 ) {

				QTableWidgetItem *celica = new QTableWidgetItem;
				if ( polja[i] == "id" ) {
					celica->setData(Qt::DisplayRole, prevedi(sql_potni_nalog.value(sql_potni_nalog.record().indexOf(polja[i])).toString()).toInt());
				}
				else if ( polja[i] == "datum_naloga" ) {
					celica->setData(Qt::DisplayRole, QDate::fromString(prevedi(sql_potni_nalog.value(sql_potni_nalog.record().indexOf(polja[i])).toString()), "dd'.'MM'.'yyyy"));
				}
				else {
					celica->setText(prevedi(sql_potni_nalog.value(sql_potni_nalog.record().indexOf(polja[i])).toString()));
				}

				if ( celica->text() != "" ) {
					ui->tbl_potni_nalogi->setItem(row, col, celica);
				}

				col++;
				i++;

			}

			row++;

		}

		ui->txt_potni_nalogi->setText(QString::number(st_nalogov, 10));

	ui->tbl_potni_nalogi->selectRow(izbranec);
	ui->tbl_potni_nalogi->sortByColumn(razvrsti, Qt::AscendingOrder);

}

void wid_osnovni_pogled::napolni_prejete_racune() {

	int izbranec = 0;
	int razvrsti = 0;

	if ( ui->tbl_prejeti_racuni->selectedItems().count() > 0 ) {
		izbranec = ui->tbl_prejeti_racuni->selectedItems().takeAt(0)->row();
	}

	razvrsti = ui->tbl_prejeti_racuni->horizontalHeader()->sortIndicatorSection();

		// podatki v vrsticah
		QString datum1 = "." + QDate::currentDate().toString("MM") + "." + QDate::currentDate().toString("yyyy");

		int st_prejetih_racunov = 0;
		double znesek_prejetih_racunov = 0.0;

		QSqlQuery sql_prejeti_racuni; // na zeljo narocnika sesteje vse, prejete v tekocem mesecu
		sql_prejeti_racuni.prepare("SELECT * FROM prejeti_racuni WHERE datum_prejema LIKE '%" + datum1 + "%'");
		sql_prejeti_racuni.exec();
		while ( sql_prejeti_racuni.next() ) {
			st_prejetih_racunov++;

			// znesek + DDV - zelja narocnika
			znesek_prejetih_racunov += prevedi(sql_prejeti_racuni.value(sql_prejeti_racuni.record().indexOf("znesek")).toString()).toDouble();
		}

		ui->txt_prejeti_racuni->setText(QString::number(st_prejetih_racunov, 10));
		ui->txt_znesek_prejetih_racunov->setText(QString::number(znesek_prejetih_racunov, 'f', 2).replace(".", ",") + " EUR");

		// podatki v tabeli
		ui->tbl_prejeti_racuni->clear();

		for (int i = 0; i <= 4; i++) {
			ui->tbl_prejeti_racuni->removeColumn(0);
		}

		QSqlQuery sql_clear;
		sql_clear.prepare("SELECT * FROM prejeti_racuni");
		sql_clear.exec();
		while (sql_clear.next()) {
			ui->tbl_prejeti_racuni->removeRow(0);
		}

		// start filling the table
		ui->tbl_prejeti_racuni->insertColumn(0);
		ui->tbl_prejeti_racuni->insertColumn(1);
		ui->tbl_prejeti_racuni->insertColumn(2);
		ui->tbl_prejeti_racuni->insertColumn(3);

		// set proper width to the first four columns

		ui->tbl_prejeti_racuni->setColumnWidth(0, 0);
		ui->tbl_prejeti_racuni->setColumnWidth(1, 100);
		ui->tbl_prejeti_racuni->setColumnWidth(2, 150);
		ui->tbl_prejeti_racuni->setColumnWidth(3, 150);

		// start filling the table
		QStringList naslovi;
		naslovi.append("ID");
		naslovi.append("Naziv podjetja");
		naslovi.append("Rok placila");
		naslovi.append("Status racunovodstva");

		ui->tbl_prejeti_racuni->setHorizontalHeaderLabels(naslovi);

		datum *delegate = new datum(this);
		ui->tbl_prejeti_racuni->setItemDelegateForColumn(2, delegate);

		int row = 0;

		sql_prejeti_racuni.clear();
		sql_prejeti_racuni.prepare("SELECT * FROM prejeti_racuni WHERE rok_placila LIKE '%" + datum1 + "%' AND status_placila NOT LIKE '" + pretvori("Pla") + "%'");
		sql_prejeti_racuni.exec();
		while ( sql_prejeti_racuni.next() ) {

			// dodaj novo vrstico v tabelo
			ui->tbl_prejeti_racuni->insertRow(row);
			ui->tbl_prejeti_racuni->setRowHeight(row, 20);

			int col = 0;
			int i = 0;

			QString polja[4] = {"id", "izdajatelj_kratki", "rok_placila", "status_racunovodstva"};

			// napolni stolpce
			while ( col < 4 ) {

				QTableWidgetItem *celica = new QTableWidgetItem;

				// vrstice z rokom placila danasnji datum napravi odebeljene
				if ( QDate::currentDate().toString("dd.MM.yyyy") == prevedi(sql_prejeti_racuni.value(sql_prejeti_racuni.record().indexOf("rok_placila")).toString()) ) {
					QFont pisava;
					pisava.setBold(true);
					celica->setFont(pisava);
				}

				if ( polja[i] == "id" ) {
					celica->setData(Qt::DisplayRole, prevedi(sql_prejeti_racuni.value(sql_prejeti_racuni.record().indexOf(polja[i])).toString()).toInt());
				}
				else if ( polja[i] == "rok_placila" ) {
					celica->setData(Qt::DisplayRole, QDate::fromString(prevedi(sql_prejeti_racuni.value(sql_prejeti_racuni.record().indexOf(polja[i])).toString()), "dd'.'MM'.'yyyy"));
				}
				else {
					celica->setText(prevedi(sql_prejeti_racuni.value(sql_prejeti_racuni.record().indexOf(polja[i])).toString()));
				}

				if ( celica->text() != "" ) {
					ui->tbl_prejeti_racuni->setItem(row, col, celica);
				}

				col++;
				i++;

			}


			row++;

		}

	ui->tbl_prejeti_racuni->selectRow(izbranec);
	ui->tbl_prejeti_racuni->sortByColumn(razvrsti, Qt::AscendingOrder);

}

void wid_osnovni_pogled::napolni_izdane_racune() {

		QString datum = "." + QDate::currentDate().toString("MM") + "." + QDate::currentDate().toString("yyyy");

		int st_predracunov = 0;
		double znesek_predracunov = 0.0;
		double znesek_avansa_predracunov = 0.0;

		int st_predplacilnih_racunov = 0;
		double znesek_predplacilnih_racunov = 0.0;

		int st_racunov = 0;
		double znesek_racunov = 0.0;

		double ddv_predplacilni = 0.0;
		double ddv_racun = 0.0;
		double ddv_predracun = 0.0;

		QSqlQuery sql_racuni;
		sql_racuni.prepare("SELECT * FROM racuni WHERE datum_izdaje LIKE '%" + datum + "%'");
		sql_racuni.exec();
		while ( sql_racuni.next() ) {

			double znesek = 0.0;
			double avans = 0.0;
			double znesek_ddv = 0.0;
			double znesek_ddv_avans = 0.0;

			QSqlQuery sql_opravila;
			sql_opravila.prepare("SELECT * FROM opravila WHERE stevilka_racuna LIKE '" + sql_racuni.value(sql_racuni.record().indexOf("id")).toString() + "'");
			sql_opravila.exec();
			while ( sql_opravila.next() ) {
				// znesek brez DDV - zelja narocnika
				znesek += prevedi(sql_opravila.value(sql_opravila.record().indexOf("znesek_koncni")).toString()).toDouble();
				znesek_ddv += prevedi(sql_opravila.value(sql_opravila.record().indexOf("znesek_ddv")).toString()).toDouble();
			}
			avans = prevedi(sql_racuni.value(sql_racuni.record().indexOf("avans")).toString()).toDouble();
			znesek_ddv_avans = prevedi(sql_racuni.value(sql_racuni.record().indexOf("avans_ddv")).toString()).toDouble();

			// predracuni
			if ( prevedi(sql_racuni.value(sql_racuni.record().indexOf("tip_racuna")).toString()) == "1" ) {
				if ( sql_racuni.value(sql_racuni.record().indexOf("status_racuna")).toString() != pretvori("Potrjen") ) {
					st_predracunov++;
					znesek_predracunov += znesek;
					ddv_predracun += znesek_ddv_avans;
				}
			}
			// predplacilni racuni
			else if ( prevedi(sql_racuni.value(sql_racuni.record().indexOf("tip_racuna")).toString()) == "2" ) {
				st_predplacilnih_racunov++;
				znesek_predplacilnih_racunov += avans;
				ddv_predplacilni += znesek_ddv_avans;
			}
			// racuni
			else if ( prevedi(sql_racuni.value(sql_racuni.record().indexOf("tip_racuna")).toString()) == "3" ) {
				st_racunov++;
				znesek_racunov += znesek;
				ddv_racun += znesek_ddv;
			}
		}

		ui->txt_predracuni->setText(QString::number(st_predracunov, 10));
		ui->txt_znesek_predracunov->setText(QString::number(znesek_predracunov, 'f', 2).replace(".", ",") + " EUR");
		ui->txt_avans_predracuna->setText(QString::number(znesek_avansa_predracunov, 'f', 2).replace(".", ",") + " EUR");

		ui->txt_predplacilni->setText(QString::number(st_predplacilnih_racunov, 10));
		ui->txt_znesek_predplacilnih->setText(QString::number(znesek_predplacilnih_racunov, 'f', 2).replace(".", ",") + " EUR");

		ui->txt_racuni->setText(QString::number(st_racunov, 10));
		ui->txt_znesek_racunov->setText(QString::number(znesek_racunov, 'f', 2).replace(".", ",") + " EUR");

		ui->txt_ddv_predracun->setText(QString::number(ddv_predracun, 'f', 2).replace(".", ",") + " EUR");
		ui->txt_ddv_predplacilni->setText(QString::number(ddv_predplacilni, 'f', 2).replace(".", ",") + " EUR");
		ui->txt_ddv_racun->setText(QString::number(ddv_racun, 'f', 2).replace(".", ",") + " EUR");

}

void wid_osnovni_pogled::on_tbl_stranke_doubleClicked() {

	projekti *uredi = new projekti;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos(ui->tbl_stranke->selectedItems().takeAt(3)->text());
	this->disconnect();

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_osnovni_pogled::on_tbl_potni_nalogi_doubleClicked() {

	potninalogi *uredi = new potninalogi;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos(ui->tbl_potni_nalogi->selectedItems().takeAt(0)->text());
	this->disconnect();

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_osnovni_pogled::on_tbl_prejeti_racuni_doubleClicked() {

	prejetiracuni *uredi = new prejetiracuni;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos(ui->tbl_prejeti_racuni->selectedItems().takeAt(0)->text());
	this->disconnect();

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_osnovni_pogled::osvezi(QString beseda) {

	if ( beseda == "stranke" ) {
		napolni_stranke();
	}
	else if ( beseda == "potninalog" ) {
		napolni_potne_naloge();
	}
	else if ( beseda == "racun" ) {
		napolni_prejete_racune();
	}

}

void wid_osnovni_pogled::napolni_predracune() {

		// clear previous content
		ui->tbl_predracun->clear();

		for (int i = 0; i <= 3; i++) {
			ui->tbl_predracun->removeColumn(0);
		}

		QSqlQuery sql_clear("wid_racuni");
		sql_clear.prepare("SELECT * FROM racuni");
		sql_clear.exec();
		while (sql_clear.next()) {
			ui->tbl_predracun->removeRow(0);
		}

		// start filling the table
		ui->tbl_predracun->insertColumn(0);
		ui->tbl_predracun->insertColumn(1);
		ui->tbl_predracun->insertColumn(2);
		ui->tbl_predracun->insertColumn(3);

		QTableWidgetItem *naslov0 = new QTableWidgetItem;
		QTableWidgetItem *naslov1 = new QTableWidgetItem;
		QTableWidgetItem *naslov2 = new QTableWidgetItem;
		QTableWidgetItem *naslov3 = new QTableWidgetItem;

		naslov0->setText("ID");
		naslov1->setText("Stranka");
		naslov2->setText("Rok placila");
		naslov3->setText("Znesek");

		ui->tbl_predracun->setHorizontalHeaderItem(0, naslov0);
		ui->tbl_predracun->setHorizontalHeaderItem(1, naslov1);
		ui->tbl_predracun->setHorizontalHeaderItem(2, naslov2);
		ui->tbl_predracun->setHorizontalHeaderItem(3, naslov3);

		ui->tbl_predracun->setColumnWidth(0, 0);
		ui->tbl_predracun->setColumnWidth(1, 150);
		ui->tbl_predracun->setColumnWidth(2, 100);
		ui->tbl_predracun->setColumnWidth(3, 100);

		datum *delegate = new datum(this);
		ui->tbl_predracun->setItemDelegateForColumn(3, delegate);

		QSqlQuery sql_fill("wid_racuni");
		sql_fill.prepare("SELECT * FROM racuni WHERE tip_racuna LIKE '1' AND status_racuna LIKE '" + pretvori("Izdan") + "'");
		sql_fill.exec();

		int row = 0;
		while (sql_fill.next()) {
			// filtriramo glede na mesec in leto
			QString filter = "pozitivno";

			// filtriramo glede na javni, zasebni status racuna
			if ( vApp->state() != pretvori("private") ) {

				// doloci vse tri cifre
				QString sklic = prevedi(sql_fill.value(sql_fill.record().indexOf("sklic")).toString());
				sklic = sklic.right(sklic.length() - 5); // odbijemo drzavo in model
				sklic = sklic.right(sklic.length() - 5); // odbijemo stevilko racuna
				int cifra_1 = sklic.left(1).toInt();
				sklic = sklic.right(sklic.length() - 3); // odbijemo cifro_1 in dan
				int cifra_2 = sklic.left(1).toInt();
				sklic = sklic.right(sklic.length() - 3); // odbijemo cifro_2 in mesec
				int cifra_3 = sklic.left(1).toInt();

				// iz prvih dveh izracunaj kontrolno stevilko
				int kontrolna = 0;

				int sestevek = 3 * cifra_1 + 2 * cifra_2;

				int ostanek = sestevek % 11;

				kontrolna = 11 - ostanek;

				if ( kontrolna >= 9 ) {
					kontrolna = 0;
				}

				// od cifre_3 odstej kontrolno stevilko
				// tako dobis 0 => racun je javen ali 1 => racun je zaseben
				int razlika = cifra_3 - kontrolna;

				if ( razlika == 1 ) {
					filter = "negativno"; // racuna ne prikazi
				}

			}

			// doloci, ali je datum ze zapadel
			QDate danes = QDate::currentDate();
			QDate rok_placila = QDate::fromString(prevedi(sql_fill.value(sql_fill.record().indexOf("rok_placila")).toString()), "dd.MM.yyyy");

			// spremenimo stranko v neplacnika
			if ( danes > rok_placila ) {
				QSqlQuery sql_placnik;
				sql_placnik.prepare("UPDATE stranke SET placilnost = 2 WHERE id LIKE '" +
									prevedi(sql_fill.value(sql_fill.record().indexOf("stranka")).toString()) + "'");
				sql_placnik.exec();
				sql_placnik.clear();
				sql_placnik.finish();
			}

			if ( filter == "pozitivno" ) {
				ui->tbl_predracun->insertRow(row);
				ui->tbl_predracun->setRowHeight(row, 20);
				int col = 0;
				int i = 0;
				QString polja[10] = {"id", "stranka", "rok_placila", "avans"};

				while (col <= 9) {
					QTableWidgetItem *celica = new QTableWidgetItem;

					// rdece obarvamo besedilo, kjer je predracun zapadel
					if ( danes > rok_placila ) {
						celica->setBackgroundColor(Qt::black);
						celica->setTextColor(Qt::red);
					}

					if ( polja[i] == "id" ) {
						celica->setData(Qt::DisplayRole, prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()).toInt());
					}
					else if ( polja[i] == "stranka" ) {
						QSqlQuery sql_find_stranka;
						sql_find_stranka.prepare("SELECT * FROM stranke WHERE id LIKE '" + sql_fill.value(sql_fill.record().indexOf(polja[i])).toString() + "'");
						sql_find_stranka.exec();
						if ( sql_find_stranka.next() ) {
							if ( prevedi(sql_find_stranka.value(sql_find_stranka.record().indexOf("tip")).toString()) == "1" ) {
								celica->setText(prevedi(sql_find_stranka.value(sql_find_stranka.record().indexOf("priimek")).toString()) + " " +
												prevedi(sql_find_stranka.value(sql_find_stranka.record().indexOf("ime")).toString()));
							}
							else {
								 celica->setText(prevedi(sql_find_stranka.value(sql_find_stranka.record().indexOf("ime")).toString()));
							}
						}
						else {
							celica->setText(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()));
						}
					}
					else if ( polja[i] == "avans" ) {
						celica->setText(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()).replace(".", ",") + " EUR");
					}
					else {
						celica->setText(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()));
					}
					ui->tbl_predracun->setItem(row, col, celica);
					col++;
					i++;
				}
				row++;
			}
		}

}

void wid_osnovni_pogled::on_tbl_predracun_doubleClicked() {

	racun *uredi = new racun;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos(ui->tbl_predracun->selectedItems().takeAt(0)->text()); // ce racun ze obstaja, naprej posljemo id. racuna

}
