#include <QtSql>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QDir>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QTextEdit>

#include "wid_racuni.h"
#include "ui_wid_racuni.h"
#include "racun.h"
#include "kodiranje.h"
#include "potninalogi.h"
#include "varnost.h"
#include "tiskanje.h"
#include "datum.h"
#include "razlog_stornacije.h"
#include "wid_dobropis.h"

wid_racuni::wid_racuni(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::wid_racuni)
{
	ui->setupUi(this);

		// disable and hide
		ui->txt_stprojekta->setEnabled(false);
		ui->txt_stprojekta->setVisible(false);
		ui->btn_brisi->setEnabled(false);
		ui->btn_brisi->setVisible(false);
		ui->btn_dobropis->setEnabled(false);
		ui->btn_dobropis->setVisible(false);

		// napolni filtrirne spustne sezname
		QString gumb = ui->btn_nov->text();
		ui->btn_nov->setText("");

			QSqlQuery sql_napolni;

			// filtriraj po tipu racuna
			ui->cb_racun->addItem("");
			ui->cb_racun->addItem("1) Predracun");
			ui->cb_racun->addItem("2) Predplacilo");
			ui->cb_racun->addItem("3) Racun");
			ui->cb_racun->addItem("4) Dobropis");

			// filtriraj po mesecu
			ui->cb_mesec->addItem("");
			ui->cb_mesec->addItem("01) Januar");
			ui->cb_mesec->addItem("02) Februar");
			ui->cb_mesec->addItem("03) Marec");
			ui->cb_mesec->addItem("04) April");
			ui->cb_mesec->addItem("05) Maj");
			ui->cb_mesec->addItem("06) Junij");
			ui->cb_mesec->addItem("07) Julij");
			ui->cb_mesec->addItem("08) Avgust");
			ui->cb_mesec->addItem("09) September");
			ui->cb_mesec->addItem("10) Oktober");
			ui->cb_mesec->addItem("11) November");
			ui->cb_mesec->addItem("12) December");
			ui->cb_mesec->addItem("Ni vnosa");

			// filtriraj po letu
			ui->cb_leto->addItem("");
			ui->cb_leto->addItem("Ni vnosa");

			sql_napolni.prepare("SELECT * FROM racuni WHERE avtor_oseba LIKE '" + pretvori(vApp->id()) + "'");
			sql_napolni.exec();
			while ( sql_napolni.next() ) {
				QString leto = prevedi(sql_napolni.value(sql_napolni.record().indexOf("datum_izdaje")).toString()).right(4);
				if ( ui->cb_leto->findText(leto) == -1 ) {
					ui->cb_leto->addItem(leto);
				}
			}
			sql_napolni.clear();

			// razvrscanje let po vrsti
			QSortFilterProxyModel* proxy = new QSortFilterProxyModel(ui->cb_leto);
			proxy->setSourceModel(ui->cb_leto->model());
			// spustni seznam prepisemo
			ui->cb_leto->model()->setParent(proxy);
			ui->cb_leto->setModel(proxy);
			// razvrsti
			ui->cb_leto->model()->sort(0);

			// privzeto izberi trenutno leto
			ui->cb_leto->setCurrentIndex(ui->cb_leto->findText(QDate::currentDate().toString("yyyy")));

			// filtriraj po statusu placila
			ui->cb_placilo->addItem("");
			sql_napolni.prepare("SELECT * FROM sif_status_placila");
			sql_napolni.exec();
			while ( sql_napolni.next() ) {
				ui->cb_placilo->addItem(prevedi(sql_napolni.value(sql_napolni.record().indexOf("status")).toString()));
			}
			sql_napolni.clear();

			// filtriraj po statusu racunovodstva
			ui->cb_racunovodstvo->addItem("");
			sql_napolni.prepare("SELECT * FROM sif_status_racunovodstva");
			sql_napolni.exec();
			while ( sql_napolni.next() ) {
				ui->cb_racunovodstvo->addItem(prevedi(sql_napolni.value(sql_napolni.record().indexOf("status")).toString()));
			}
			sql_napolni.clear();

		napolni();

		ui->btn_nov->setText(gumb);

		if ( vApp->permission() == pretvori("Administrator") ) {
			ui->btn_brisi->setVisible(true);
			ui->btn_brisi->setEnabled(true);
		}
		else {
			ui->btn_brisi->setVisible(false);
			ui->btn_brisi->setEnabled(false);
		}
}

wid_racuni::~wid_racuni()
{
	delete ui;
}

void wid_racuni::on_cb_racun_currentIndexChanged() {

	ui->btn_dobropis->setEnabled(false);
	ui->btn_dobropis->setVisible(false);
	ui->btn_storno->setEnabled(false);

	if ( ui->cb_racun->currentText().left(3) == "3) " ) {
		ui->btn_storno->setEnabled(true);
		ui->btn_dobropis->setEnabled(true);
		ui->btn_dobropis->setVisible(true);
	}
	else if ( ui->cb_racun->currentText().left(3) == "4) " ) {
	}
	else {
		ui->btn_storno->setEnabled(false);
	}

	if ( ui->btn_nov->text() != "" ) {
		napolni();
	}

}

void wid_racuni::on_cb_mesec_currentIndexChanged() {

	if (ui->cb_mesec->currentText() == "Ni vnosa" ) {
		ui->cb_leto->setCurrentText("Ni vnosa");
	}
	if ( ui->btn_nov->text() != "" ) {
		napolni();
	}

}

void wid_racuni::on_cb_leto_currentIndexChanged() {

	if (ui->cb_leto->currentText() == "Ni vnosa" ) {
		ui->cb_mesec->setCurrentText("Ni vnosa");
	}
	if ( ui->btn_nov->text() != "" ) {
		napolni();
	}

}

void wid_racuni::on_cb_placilo_currentIndexChanged() {

	if ( ui->btn_nov->text() != "" ) {
		napolni();
	}

}

void wid_racuni::on_cb_racunovodstvo_currentIndexChanged() {

	if ( ui->btn_nov->text() != "" ) {
		napolni();
	}

}

void wid_racuni::napolni() {

	ui->btn_print->setText("Polnim");

	int izbranec = 0;
	int razvrsti = 0;

	if ( ui->tbl_racuni->selectedItems().count() > 0 ) {
		izbranec = ui->tbl_racuni->selectedItems().takeAt(0)->row();
	}

	razvrsti = ui->tbl_racuni->horizontalHeader()->sortIndicatorSection();

		// clear previous content
		ui->tbl_racuni->clear();

		for (int i = 0; i <= 11; i++) {
			ui->tbl_racuni->removeColumn(0);
		}

		QSqlQuery sql_clear("wid_racuni");
		sql_clear.prepare("SELECT * FROM racuni");
		sql_clear.exec();
		while (sql_clear.next()) {
			ui->tbl_racuni->removeRow(0);
		}

		// start filling the table
		ui->tbl_racuni->insertColumn(0);
		ui->tbl_racuni->insertColumn(1);
		ui->tbl_racuni->insertColumn(2);
		ui->tbl_racuni->insertColumn(3);
		ui->tbl_racuni->insertColumn(4);
		ui->tbl_racuni->insertColumn(5);
		ui->tbl_racuni->insertColumn(6);
		ui->tbl_racuni->insertColumn(7);
		ui->tbl_racuni->insertColumn(8);
		ui->tbl_racuni->insertColumn(9);
		ui->tbl_racuni->insertColumn(10);
		ui->tbl_racuni->insertColumn(11);

		QTableWidgetItem *naslov0 = new QTableWidgetItem;
		QTableWidgetItem *naslov1 = new QTableWidgetItem;
		QTableWidgetItem *naslov2 = new QTableWidgetItem;
		QTableWidgetItem *naslov3 = new QTableWidgetItem;
		QTableWidgetItem *naslov4 = new QTableWidgetItem;
		QTableWidgetItem *naslov5 = new QTableWidgetItem;
		QTableWidgetItem *naslov6 = new QTableWidgetItem;
		QTableWidgetItem *naslov7 = new QTableWidgetItem;
		QTableWidgetItem *naslov8 = new QTableWidgetItem;
		QTableWidgetItem *naslov9 = new QTableWidgetItem;
		QTableWidgetItem *naslov10 = new QTableWidgetItem;
		QTableWidgetItem *naslov11 = new QTableWidgetItem;

		naslov0->setText("ID");
		naslov1->setText("Tip racuna");
		naslov2->setText("St. racuna");
		naslov3->setText("Datum izdaje");
		naslov4->setText("Stranka");
		naslov5->setText("Projekt");
		naslov6->setText("Znesek za placilo");
		naslov7->setText("Se placati");
		naslov8->setText("Status placila");
		naslov9->setText("Status racunovodstva");
		naslov10->setText("Stornacija");
		naslov11->setText("Dobropis");

		ui->tbl_racuni->setHorizontalHeaderItem(0, naslov0);
		ui->tbl_racuni->setHorizontalHeaderItem(1, naslov1);
		ui->tbl_racuni->setHorizontalHeaderItem(2, naslov2);
		ui->tbl_racuni->setHorizontalHeaderItem(3, naslov3);
		ui->tbl_racuni->setHorizontalHeaderItem(4, naslov4);
		ui->tbl_racuni->setHorizontalHeaderItem(5, naslov5);
		ui->tbl_racuni->setHorizontalHeaderItem(6, naslov6);
		ui->tbl_racuni->setHorizontalHeaderItem(7, naslov7);
		ui->tbl_racuni->setHorizontalHeaderItem(8, naslov8);
		ui->tbl_racuni->setHorizontalHeaderItem(9, naslov9);
		ui->tbl_racuni->setHorizontalHeaderItem(10, naslov10);
		ui->tbl_racuni->setHorizontalHeaderItem(11, naslov11);

		ui->tbl_racuni->setColumnWidth(0, 0);

		datum *delegate = new datum(this);
		ui->tbl_racuni->setItemDelegateForColumn(3, delegate);

		QString projekt = "";

		QSqlQuery sql_projekt;
		sql_projekt.prepare("SELECT * FROM projekti WHERE id LIKE '" + pretvori(ui->txt_stprojekta->text()) + "'");
		sql_projekt.exec();
		if ( sql_projekt.next() ) {
			projekt = sql_projekt.value(sql_projekt.record().indexOf("id")).toString();
		}

		QString stavek = "";

		if ( ui->cb_racun->currentText() != "" ) {
			stavek += " AND tip_racuna LIKE '" + pretvori(ui->cb_racun->currentText()).left(1) + "'";
		}
		if ( ui->cb_placilo->currentText() != "" ) {
			stavek += " AND status_placila LIKE '" + pretvori (ui->cb_placilo->currentText()) + "'";
		}
		if ( ui->cb_racunovodstvo->currentText() != "" ) {
			stavek += " AND status_racunovodstva LIKE '" + pretvori (ui->cb_racunovodstvo->currentText()) + "'";
		}

		if ( stavek != "" && ui->txt_stprojekta->text() == "*" && stavek.indexOf(" WHERE") == -1 ) {
			stavek = " WHERE" + stavek.right(stavek.length() - 4);
		}

		QSqlQuery sql_fill("wid_racuni");
		if ( ui->txt_stprojekta->text() != "*" ) {
			sql_fill.prepare("SELECT * FROM racuni WHERE projekt LIKE '" + projekt + "'" + stavek);
		}
		else {
			sql_fill.prepare("SELECT * FROM racuni" + stavek);
		}
		sql_fill.exec();

		int row = 0;
		while (sql_fill.next()) {
			// filtriramo glede na mesec in leto
			QString filter = "pozitivno";
			if ( ui->cb_mesec->currentText() == "Ni vnosa" && ui->cb_leto->currentText() == "Ni vnosa" ) {
				if ( sql_fill.value(sql_fill.record().indexOf("datum_izdaje")).toString() != "" ) {
					filter = "negativno";
				}
			}
			else if ( ui->cb_mesec->currentText() != "" && ui->cb_leto->currentText() != "" ) {
				QString leto = prevedi(sql_fill.value(sql_fill.record().indexOf("datum_izdaje")).toString()).right(4);
				QString mesec = prevedi(sql_fill.value(sql_fill.record().indexOf("datum_izdaje")).toString()).left(5).right(2);
				if ( mesec != ui->cb_mesec->currentText().left(2) || leto != ui->cb_leto->currentText() ) {
					filter = "negativno";
				}
			}
			else if ( ui->cb_mesec->currentText() != "" ) {
				QString mesec = prevedi(sql_fill.value(sql_fill.record().indexOf("datum_izdaje")).toString()).left(5).right(2);
				if ( mesec != ui->cb_mesec->currentText().left(2) ) {
					filter = "negativno";
				}
			}
			else if ( ui->cb_leto->currentText() != "" ) {
				QString leto = prevedi(sql_fill.value(sql_fill.record().indexOf("datum_izdaje")).toString()).right(4);
				if ( leto != ui->cb_leto->currentText() ) {
					filter = "negativno";
				}
			}
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

			if ( filter == "pozitivno" ) {
				ui->tbl_racuni->insertRow(row);
				ui->tbl_racuni->setRowHeight(row, 20);
				int col = 0;
				int i = 0;
				QString polja[12] = {"id", "tip_racuna", "stevilka_racuna", "datum_izdaje", "stranka", "projekt", "znesek_za_placilo",
									 "se_placati", "status_placila", "status_racunovodstva", "stornacija", "dobropis"};

				while (col <= 11) {
					QTableWidgetItem *celica = new QTableWidgetItem;

					// storniranim/dobropisanim vrsticam spremenimo barvo ozadja in besedila
					if ( prevedi(sql_fill.value(sql_fill.record().indexOf("stornacija")).toString()) == "1" ) {
						celica->setBackgroundColor(Qt::red);
						celica->setTextColor(Qt::black);
					}
					if ( prevedi(sql_fill.value(sql_fill.record().indexOf("dobropis")).toString()) == "1" ) {
						celica->setBackgroundColor(Qt::blue);
						celica->setTextColor(Qt::white);
					}

					if ( polja[i] == "id" ) {
						celica->setData(Qt::DisplayRole, prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()).toInt());
					}
					else if ( polja[i] == "stornacija" ) {
						if ( prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()) == "1" ) { // racun je storniran
							celica->setText("DA");
						}
						else if ( prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()) == "0" ) { // racun ni storniran
							celica->setText("NE");
						}
						else { // ne gre za racun ampak za predracun, predplacilni racun ali stornacijo
							celica->setText("");
						}
					}
					else if ( polja[i] == "dobropis" ) {
						if ( prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()) == "1" ) { // racun je storniran
							celica->setText("DA");
						}
						else if ( prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()) == "0" ) { // racun ni storniran
							celica->setText("NE");
						}
						else { // ne gre za racun ampak za predracun, predplacilni racun ali stornacijo
							celica->setText("");
						}
					}
					else if ( polja[i] == "datum_izdaje" ) {
						celica->setData(Qt::DisplayRole, QDate::fromString(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()), "dd'.'MM'.'yyyy"));
					}
					else if ( polja[i] == "tip_racuna" ) {
						if ( prevedi(sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString()) == "1" ) {
							celica->setText("Predracun");
						}
						else if ( prevedi(sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString()) == "2" ) {
							celica->setText("Predplacilo");
						}
						else if ( prevedi(sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString()) == "3" ) {
							celica->setText("Racun");
						}
						else if ( prevedi(sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString()) == "4" ) {
							celica->setText("Dobropis");
						}
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
					else if ( polja[i] == "projekt" ) {
						QSqlQuery sql_kodiraj;
						sql_kodiraj.prepare("SELECT * FROM projekti WHERE id LIKE '" + sql_fill.value(sql_fill.record().indexOf("projekt")).toString() + "'");
						sql_kodiraj.exec();
						if ( sql_kodiraj.next() ) {
							celica->setText(prevedi(sql_kodiraj.value(sql_kodiraj.record().indexOf("naslov_projekta")).toString()));
						}
					}
					else if ( polja[i] == "znesek_za_placilo" ) {
						QSqlQuery sql_kodiraj;
						sql_kodiraj.prepare("SELECT * FROM opravila WHERE stevilka_racuna LIKE '" + sql_fill.value(sql_fill.record().indexOf("id")).toString() +
																"' AND tip_racuna LIKE '" + sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString() + "'");
						sql_kodiraj.exec();
						double znesek = 0.0;
						while ( sql_kodiraj.next() ) {
							znesek += prevedi(sql_kodiraj.value(sql_kodiraj.record().indexOf("znesek_koncni")).toString()).toDouble() *
									( prevedi(sql_kodiraj.value(sql_kodiraj.record().indexOf("ddv")).toString()).toDouble() + 100.0 ) / 100.0;
						}
						celica->setText(QString::number(znesek, 'f', 2).replace(".", ",") + " EUR");
					}
					else if ( polja[i] == "se_placati" ) {
						QSqlQuery sql_kodiraj;
						sql_kodiraj.prepare("SELECT * FROM opravila WHERE stevilka_racuna LIKE '" + sql_fill.value(sql_fill.record().indexOf("id")).toString() +
																"' AND tip_racuna LIKE '" + sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString() + "'");
						sql_kodiraj.exec();
						double znesek = 0.0;
						while ( sql_kodiraj.next() ) {
							znesek += prevedi(sql_kodiraj.value(sql_kodiraj.record().indexOf("znesek_koncni")).toString()).toDouble() *
									  ( prevedi(sql_kodiraj.value(sql_kodiraj.record().indexOf("ddv")).toString()).toDouble() + 100.0 ) / 100.0;
						}
						znesek -= prevedi(sql_fill.value(sql_fill.record().indexOf("avans")).toString()).toDouble();
						celica->setText(QString::number(znesek, 'f', 2).replace(".", ",") + " EUR");
					}
					else {
						celica->setText(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()));
					}

					ui->tbl_racuni->setItem(row, col, celica);
					col++;
					i++;
				}
				row++;
			}
		}

	ui->tbl_racuni->selectRow(izbranec);
	ui->tbl_racuni->sortByColumn(razvrsti, Qt::AscendingOrder);

	ui->btn_print->setText("Natisni");

}

void wid_racuni::napolni_sorodnike() {

	int izbranec = 0;
	int razvrsti = 0;

	if ( ui->tbl_sorodniki->selectedItems().count() > 0 ) {
		izbranec = ui->tbl_sorodniki->selectedItems().takeAt(0)->row();
	}

	razvrsti = ui->tbl_sorodniki->horizontalHeader()->sortIndicatorSection();

		// clear previous content
		ui->tbl_sorodniki->clear();

		for (int i = 0; i <= 3; i++) {
			ui->tbl_sorodniki->removeColumn(0);
		}

		QSqlQuery sql_clear("wid_sorodniki");
		sql_clear.prepare("SELECT * FROM racuni");
		sql_clear.exec();
		while (sql_clear.next()) {
			ui->tbl_sorodniki->removeRow(0);
		}

		// start filling the table
		ui->tbl_sorodniki->insertColumn(0);
		ui->tbl_sorodniki->insertColumn(1);
		ui->tbl_sorodniki->insertColumn(2);

		QTableWidgetItem *naslov0 = new QTableWidgetItem;
		QTableWidgetItem *naslov1 = new QTableWidgetItem;
		QTableWidgetItem *naslov2 = new QTableWidgetItem;

		naslov0->setText("ID");
		naslov1->setText("Tip racuna");
		naslov2->setText("St. racuna");

		ui->tbl_sorodniki->setHorizontalHeaderItem(0, naslov0);
		ui->tbl_sorodniki->setHorizontalHeaderItem(1, naslov1);
		ui->tbl_sorodniki->setHorizontalHeaderItem(2, naslov2);

		ui->tbl_sorodniki->setColumnWidth(0, 35);
		ui->tbl_sorodniki->setColumnWidth(1, 70);
		ui->tbl_sorodniki->setColumnWidth(2, 70);

		// dolocimo ID, po katerem iscemo sorodnike ter ID, ki je trenutno izbran
		QString id_starsa = "";
		QString id_izbran = "";

		id_izbran = ui->tbl_racuni->selectedItems().takeAt(0)->text();

		if ( ui->tbl_racuni->selectedItems().takeAt(1)->text().left(6) == "Predra" ) { // predracun, id_izbran == id_starsa
			id_starsa = id_izbran;
		}
		else { // predplacilni racun ali racun
			QSqlQuery sql_stars;
			sql_stars.prepare("SELECT * FROM racuni WHERE id LIKE '" + pretvori(id_izbran) + "'");
			sql_stars.exec();
			if ( sql_stars.next() ) {
				id_starsa = prevedi(sql_stars.value(sql_stars.record().indexOf("stevilka_starsa")).toString());
			}
		}

		QString sql_stavek = "SELECT * FROM racuni WHERE";
		sql_stavek.append(" stevilka_starsa LIKE '" + pretvori(id_starsa) + "'");
		sql_stavek.append(" OR id LIKE '" + pretvori(id_starsa) + "'");

		QSqlQuery sql_fill("wid_sorodniki");
		sql_fill.prepare(sql_stavek);
		sql_fill.exec();

		int row = 0;
		while (sql_fill.next()) {
			// izlocimo trenutno izbran racun
			if ( id_izbran != prevedi(sql_fill.value(sql_fill.record().indexOf("id")).toString()) ) {
				ui->tbl_sorodniki->insertRow(row);
				ui->tbl_sorodniki->setRowHeight(row, 20);
				int col = 0;
				int i = 0;
				QString polja[3] = {"id", "tip_racuna", "stevilka_racuna"};

				while (col <= 2) {
					QTableWidgetItem *celica = new QTableWidgetItem;
					if ( polja[i] == "id" ) {
						celica->setData(Qt::DisplayRole, prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()).toInt());
					}
					else if ( polja[i] == "tip_racuna" ) {
						if ( prevedi(sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString()) == "1" ) {
							celica->setText("Predracun");
						}
						else if ( prevedi(sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString()) == "2" ) {
							celica->setText("Predplacilo");
						}
						else if ( prevedi(sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString()) == "3" ) {
							celica->setText("Racun");
						}
						else if ( prevedi(sql_fill.value(sql_fill.record().indexOf("tip_racuna")).toString()) == "4" ) {
							celica->setText("Stornacija");
						}
					}
					else {
						celica->setText(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()));
					}
					ui->tbl_sorodniki->setItem(row, col, celica);
					col++;
					i++;
				}
				row++;
			}
		}

	ui->tbl_sorodniki->selectRow(izbranec);
	ui->tbl_sorodniki->sortByColumn(razvrsti, Qt::AscendingOrder);

}

void wid_racuni::on_tbl_racuni_doubleClicked() {

	racun *uredi = new racun;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos(ui->tbl_racuni->selectedItems().takeAt(0)->text()); // ce racun ze obstaja, naprej posljemo id. racuna
	this->disconnect();

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_racuni::on_tbl_racuni_itemSelectionChanged() {

	if ( ui->btn_print->text() != "Polnim" ) {
		if ( ui->tbl_racuni->selectedItems().count() == 12 ) { // pri enem oznacenem polju omogoci gumbe, pri vecih pa ne
			napolni_sorodnike();
			ui->btn_storno->setEnabled(true);
			ui->btn_dobropis->setEnabled(true);

			// preverimo, ce gre res za racun
			if ( ui->tbl_racuni->selectedItems().takeAt(1)->text() == "Racun" ) {
				// stornacije so mozne samo ce racun se ni storniran
				if ( ui->tbl_racuni->selectedItems().takeAt(10)->text() != "NE" &&  ui->tbl_racuni->selectedItems().takeAt(11)->text() != "NE" ) {
					ui->btn_storno->setEnabled(false);
					ui->btn_dobropis->setEnabled(false);
				}
				else {
					ui->btn_storno->setEnabled(true);
					ui->btn_dobropis->setEnabled(true);
				}
			}
			else {
				ui->btn_storno->setEnabled(false);
				ui->btn_dobropis->setEnabled(false);
			}
		}
		else {
			ui->tbl_sorodniki->clear();
			ui->btn_storno->setEnabled(false);
			ui->btn_dobropis->setEnabled(false);
		}

	}

}

void wid_racuni::on_tbl_sorodniki_doubleClicked() {

	racun *uredi = new racun;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos(ui->tbl_sorodniki->selectedItems().takeAt(0)->text()); // ce racun ze obstaja, naprej posljemo id. racuna
	this->disconnect();

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_racuni::on_btn_brisi_clicked() {

	QString id = ui->tbl_racuni->selectedItems().takeAt(0)->text();

		QSqlQuery sql_brisi;

		// ob brisanju dobropisa spremeni status dobropisa pri racunu na neobstojec
		QSqlQuery sql_st_racuna;
		sql_st_racuna.prepare("SELECT * FROM racuni WHERE id LIKE '" + pretvori(id) + "'");
		sql_st_racuna.exec();
		if ( sql_st_racuna.next() ) {
			if ( sql_st_racuna.value(sql_st_racuna.record().indexOf("tip_racuna")).toString() == "4" ) { // brisemo dobropis
				sql_brisi.prepare("UPDATE racuni SET 'dobropis' = '0' WHERE id LIKE '" + sql_st_racuna.value(sql_st_racuna.record().indexOf("stevilka_starsa")).toString()  + "'");
				sql_brisi.exec();
				sql_brisi.clear();
			}
		}
		sql_st_racuna.clear();

		// izbrisi opravila
		sql_brisi.prepare("DELETE FROM opravila WHERE stevilka_racuna LIKE '" + pretvori(id) + "'");
		sql_brisi.exec();
		sql_brisi.clear();

		// izbrisi zapise
		sql_brisi.prepare("DELETE FROM opombe WHERE stevilka_racuna LIKE '" + pretvori(id) + "'");
		sql_brisi.exec();
		sql_brisi.clear();

		// izbrisi racun
		sql_brisi.prepare("DELETE FROM racuni WHERE id LIKE '" + pretvori(id) + "'");
		sql_brisi.exec();

	ui->tbl_racuni->removeRow(ui->tbl_racuni->selectedItems().takeAt(0)->row());
	osvezi("racuni");

}

void wid_racuni::on_btn_kopiraj_clicked() {

	QClipboard *odlozisce = QApplication::clipboard();

	QModelIndexList selectedList = ui->tbl_racuni->selectionModel()->selectedRows();

	QString html_besedilo = "<table>";
	html_besedilo += "<tr>";
	html_besedilo += "<th>ID</th>";
	html_besedilo += "<th>Tip racuna</th>";
	html_besedilo += "<th>Stevilka racuna</th>";
	html_besedilo += "<th>Datum izdaje</th>";
	html_besedilo += "<th>Stranka</th>";
	html_besedilo += "<th>Projekt</th>";
	html_besedilo += "<th>Znesek za placilo</th>";
	html_besedilo += "<th>Se placati</th>";
	html_besedilo += "<th>Status placila</th>";
	html_besedilo += "<th>Status racunovodstva</th>";
	html_besedilo += "</tr>";

	for( int i = 0; i < selectedList.count(); i++) {
		html_besedilo += "<tr>";
		for ( int a = 0; a < 10; a++ ) {
			html_besedilo += "<td>";
			html_besedilo += ui->tbl_racuni->item(selectedList.at(i).row(), a)->text();
			html_besedilo += "</td>";

		}
		html_besedilo += "</tr>";
	}

	html_besedilo += "</table>";

	QTextEdit *textedit = new QTextEdit;

	textedit->setHtml(html_besedilo);
	html_besedilo = textedit->toHtml();

	odlozisce->clear();

	QMimeData *mimeData = new QMimeData();
	mimeData->setData("text/html", html_besedilo.toUtf8());
	odlozisce->setMimeData(mimeData, QClipboard::Clipboard);


}

void wid_racuni::on_btn_storno_clicked() {

	if ( ui->tbl_racuni->selectedItems().takeAt(10)->text() == "DA" ) {
		ui->btn_storno->setEnabled(false);
	}
	else {
		QString id = ui->tbl_racuni->selectedItems().takeAt(0)->text();

			// povprasaj za razlog stornacije, sele nato ga izvedi
			razlog_stornacije *vnesi_razlog = new razlog_stornacije;
			vnesi_razlog->show();
			QObject::connect(this, SIGNAL(prenos(QString)),
					   vnesi_razlog , SLOT(prejem(QString)));
			prenos(ui->tbl_racuni->selectedItems().takeAt(0)->text()); // ce racun ze obstaja, naprej posljemo id. racuna
			this->disconnect();

			// receive signal to refresh table
			QObject::connect(vnesi_razlog, SIGNAL(poslji(QString)),
					   this , SLOT(osvezi(QString)));

		osvezi("racuni");

		napolni_sorodnike();
	}

}


void wid_racuni::on_btn_dobropis_clicked() {

		// na novo izracunamo stevilko dobropisa
		QString stevilka_dobropisa = stevilka_racuna("4");

		// prepisemo podatke iz racuna v dobropis
		QSqlQuery sql_vnesi_projekt;
		sql_vnesi_projekt.prepare("INSERT INTO racuni (stevilka_racuna, tip_racuna, status_racuna, stranka, projekt, avtor_oseba, datum_pricetka, "
								  "datum_konca, datum_izdaje, datum_placila, status_placila, status_racunovodstva, avans, odstotek_avansa, "
								  "status_oddaje_racuna, datum_oddaje_racuna, stara_stevilka_racuna, sklic, datum_placila_avansa, opombe, "
								  "rok_placila, podjetje_id, podjetje_kratki, podjetje_polni, podjetje_naslov_ulica, podjetje_naslov_stevilka, "
								  "podjetje_naslov_posta, podjetje_naslov_postna_stevilka, podjetje_url, podjetje_email, podjetje_telefon, podjetje_ddv, "
								  "podjetje_bic, podjetje_banka, podjetje_tekoci_racun, podjetje_koda_namena, podjetje_logotip, izdajatelj_id, "
								  "izdajatelj_ime, izdajatelj_priimek, izdajatelj_naziv, narocnik_id, narocnik_naziv, narocnik_naslov, narocnik_posta, narocnik_davcna, "
								  "stevilka_starsa, stornacija) "
								  "SELECT '" + stevilka_dobropisa + "', '4', status_racuna, stranka, projekt, avtor_oseba, datum_pricetka, "
								  "datum_konca, datum_izdaje, datum_placila, status_placila, status_racunovodstva, avans, odstotek_avansa, "
								  "status_oddaje_racuna, datum_oddaje_racuna, stara_stevilka_racuna, sklic, datum_placila_avansa, opombe, "
								  "rok_placila, podjetje_id, podjetje_kratki, podjetje_polni, podjetje_naslov_ulica, podjetje_naslov_stevilka, "
								  "podjetje_naslov_posta, podjetje_naslov_postna_stevilka, podjetje_url, podjetje_email, podjetje_telefon, podjetje_ddv, "
								  "podjetje_bic, podjetje_banka, podjetje_tekoci_racun, podjetje_koda_namena, podjetje_logotip, izdajatelj_id, "
								  "izdajatelj_ime, izdajatelj_priimek, izdajatelj_naziv, narocnik_id, narocnik_naziv, narocnik_naslov, narocnik_posta, narocnik_davcna, "
								  "id, stornacija FROM racuni "
								  "WHERE id LIKE '" + pretvori(ui->tbl_racuni->selectedItems().takeAt(0)->text())  + "'");
		sql_vnesi_projekt.exec();

		// na racunu oznacimo, da gre za dobropis
		QSqlQuery sql_oznaci_dobropis;
		sql_oznaci_dobropis.prepare("UPDATE racuni SET 'dobropis' = '1' WHERE id LIKE '" + pretvori(ui->tbl_racuni->selectedItems().takeAt(0)->text())  + "'");
		sql_oznaci_dobropis.exec();

		// poiscemo id pravkar vnesenega zapisa
		QSqlQuery sql_nov_id;
		QString nov_id = "";
		sql_nov_id.prepare("SELECT * FROM racuni WHERE tip_racuna LIKE '" + pretvori("4") + "' ORDER BY id ASC");
		sql_nov_id.exec();
		while ( sql_nov_id.next() ) {
			nov_id = prevedi(sql_nov_id.value(sql_nov_id.record().indexOf("id")).toString());
		}

		// kopira opravila iz racuna v dobropis
		QSqlQuery sql_kopiraj_opravila;
		sql_kopiraj_opravila.prepare("INSERT INTO opravila (stevilka_stranke, stevilka_projekta, stevilka_racuna, tip_racuna, opravilo_skupina, "
									 "opravilo_storitev, urna_postavka_brez_ddv, urna_postavka_z_ddv, ddv, popust_fb1, popust_fb2, "
									 "popust_komb1, popust_komb2, popust_stranka, popust_akcija, podrazitev_vikend, "
									 "podrazitev_hitrost, podrazitev_zapleti, pribitek_vikend, pribitek_hitrost, pribitek_zapleti, "
									 "tip_ur, ur_dela, rocni_vnos_ur, znesek_popustov, znesek_ddv, znesek_koncni, enota, opravilo_sklop, "
									 "opravilo_rocno, vrstni_red, sifra) "
									 "SELECT stevilka_stranke, stevilka_projekta, '" + nov_id + "', '4', opravilo_skupina, "
									 "opravilo_storitev, urna_postavka_brez_ddv, urna_postavka_z_ddv, ddv, popust_fb1, popust_fb2, "
									 "popust_komb1, popust_komb2, popust_stranka, popust_akcija, podrazitev_vikend, "
									 "podrazitev_hitrost, podrazitev_zapleti, pribitek_vikend, pribitek_hitrost, pribitek_zapleti, "
									 "tip_ur, ur_dela, rocni_vnos_ur, znesek_popustov, znesek_ddv, znesek_koncni, enota, opravilo_sklop, "
									 "opravilo_rocno, vrstni_red, sifra FROM opravila "
									 "WHERE stevilka_racuna LIKE '" + pretvori(ui->tbl_racuni->selectedItems().takeAt(0)->text()) + "' AND tip_racuna LIKE '3'");
		sql_kopiraj_opravila.exec();

	// prestavi tabelo na pravkar tvorjen dobropis
	ui->cb_racun->setCurrentIndex(ui->cb_racun->findText("4) ", Qt::MatchStartsWith));
	ui->tbl_racuni->selectRow(ui->tbl_racuni->rowCount() - 1);

	on_tbl_racuni_doubleClicked();

}

QString wid_racuni::stevilka_racuna(QString tip) {

	QString st_racuna = "";

		QString leto = QDate::currentDate().toString("yyyy");
		QString mesec = QDate::currentDate().toString("MM");
		QString dan = QDate::currentDate().toString("dd");

		int max_st_racuna = 0;

		// izracunamo zaporedno stevilko racuna v tekocem letu
		QSqlQuery sql_stetje_racunov;
		sql_stetje_racunov.prepare("SELECT * FROM racuni WHERE datum_izdaje LIKE '%." + pretvori(leto) +
								   "' AND tip_racuna LIKE '" + pretvori(tip) + "' ORDER BY stevilka_racuna ASC");
		sql_stetje_racunov.exec();
		while ( sql_stetje_racunov.next() ) {
			int st_racuna = 0;
			st_racuna = prevedi(sql_stetje_racunov.value(sql_stetje_racunov.record().indexOf("stevilka_racuna")).toString()).right(3).toInt();
			if ( st_racuna > max_st_racuna ) {
				max_st_racuna = st_racuna;
			}
		}

		max_st_racuna = max_st_racuna + 1;
		st_racuna = QString::number(max_st_racuna, 10);

		// iz stevilke racuna ustvarimo tromestno stevilko, pretvorjeno v besedo
		if ( st_racuna.length() == 1 ) {
			st_racuna = "00" + st_racuna;
		}
		else if ( st_racuna.length() == 2 ) {
			st_racuna = "0" + st_racuna;
		}

		QString predpona = "";

		if ( tip == "1" ) {
			predpona = "P";
		}
		else if ( tip == "2" ) {
			predpona = "A";
		}
		else if ( tip == "3" ) {
			predpona = "R";
		}
		else if ( tip == "4" ) {
			predpona = "D";
		}
		// imamo dovolj podatkov za tvorbo stevilke racuna

		st_racuna = predpona + QDate::currentDate().toString("yyyy").right(2) + st_racuna;

	return st_racuna;

}

void wid_racuni::osvezi(QString beseda) {

	if ( beseda == "racuni" ) {
		napolni();
		prenesi();
	}

}

QString wid_racuni::pretvori(QString besedilo) {

	return kodiranje().zakodiraj(besedilo);

}

QString wid_racuni::prevedi(QString besedilo) {

	return kodiranje().odkodiraj(besedilo);

}

void wid_racuni::on_btn_nov_clicked() {

	racun *uredi = new racun;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos("Nov racun" + ui->txt_stprojekta->text()); // ce racun ne obstaja, naprej posljemo st. projekta
	this->disconnect();

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_racuni::prejem(QString besedilo) {

	ui->txt_stprojekta->setText(besedilo);

	napolni();

}

void wid_racuni::tip(QString besedilo) {

	ui->txt_stprojekta->setText("*");

	ui->cb_racun->setCurrentIndex(ui->cb_racun->findText(besedilo + ") ", Qt::MatchStartsWith));

//    napolni();

}

void wid_racuni::on_btn_refresh_clicked() {

	napolni();

}

void wid_racuni::on_btn_print_clicked() {

	QString seznam_id = "";
	QModelIndexList selectedList = ui->tbl_racuni->selectionModel()->selectedRows();

	for( int i = 0; i < selectedList.count(); i++) {
		if ( ui->tbl_racuni->item(selectedList.at(i).row(), 2)->text() != "" ) {
			seznam_id.append(ui->tbl_racuni->item(selectedList.at(i).row(), 0)->text() + ",");
		}
	}

	// posljemo v tisk
	tiskanje *print = new tiskanje;
	print->show();
	QObject::connect(this, SIGNAL(tiskaj(QString, QString, QString)),
				 print , SLOT(prejem(QString, QString, QString)));
	tiskaj("izdani-racuni", seznam_id, "tisk"); // vrsta, id stevilke, format

}

void wid_racuni::on_btn_print_pdf_clicked() {

	QString seznam_id = "";
	QModelIndexList selectedList = ui->tbl_racuni->selectionModel()->selectedRows();

	for( int i = 0; i < selectedList.count(); i++) {
		if ( ui->tbl_racuni->item(selectedList.at(i).row(), 2)->text() != "" ) {
			seznam_id.append(ui->tbl_racuni->item(selectedList.at(i).row(), 0)->text() + ",");
		}
	}

	// posljemo v tisk
	tiskanje *print = new tiskanje;
	print->show();
	QObject::connect(this, SIGNAL(tiskaj(QString, QString, QString)),
				 print , SLOT(prejem(QString, QString, QString)));
	tiskaj("izdani-racuni", seznam_id, "pdf"); // vrsta, id stevilke, format

}

QString wid_racuni::pretvori_v_double(QString besedilo) {

	besedilo.remove(" ");
	besedilo.remove("%");
	besedilo.remove("EUR");
	besedilo.replace(",", ".");

	if ( besedilo.left(1) == "0" ) {
		besedilo.remove(0,1);
	}
	if ( besedilo == "" ) { // ce je polje prazno, dodamo vrednost 0.0
		besedilo.append("0.0");
	}
	if ( besedilo.left(1) == "." ) { // ce besedilo nima vodilne nicle, pa je pricakovana, jo dodamo
		besedilo.prepend("0");
	}
	if ( besedilo.right(1) == "." ) { // ce ima besedilo decimalno locilo, za njim pa nic, dodamo 0
		besedilo.append("0");
	}
	if ( !besedilo.contains(".") ) { // ce je celo stevilo dodamo decimalno locilo in vrednost 0
		besedilo.append(".0");
	}

	return besedilo;

}

QString wid_racuni::pretvori_iz_double(QString besedilo) {

	besedilo.replace(".",",");

	return besedilo;

}

void wid_racuni::on_btn_print_seznam_clicked() {

}

void wid_racuni::on_btn_prestevilci_clicked() {

	QString gumb = ui->btn_prestevilci->text();
	ui->btn_prestevilci->setText("Izvajam prestevilcenje racunov.");
	qApp->processEvents();

		int i_vseh_racunov = 0;
		int i_zaporedna = 0;

		QSqlQuery sql_prestej_racune;
		sql_prestej_racune.prepare("SELECT * FROM racuni WHERE stevilka_racuna NOT LIKE ''");
		sql_prestej_racune.exec();
		while ( sql_prestej_racune.next() ) {
			i_vseh_racunov++;
		}

		// naredi prestevilcenje za vse tipe racunov
		for ( int i_tip_racuna = 1; i_tip_racuna <= 3; i_tip_racuna++ ) {

			// poisci vsa leta; dobimo seznam vseh let, v katerih smo uradovali
			QStringList leta;

			QSqlQuery sql_leta;
			sql_leta.prepare("SELECT * FROM racuni WHERE tip_racuna LIKE '" + pretvori(QString::number(i_tip_racuna, 10)) +
							 "' AND stevilka_racuna NOT LIKE ''");
			sql_leta.exec();
			while ( sql_leta.next() ) {
				if ( !leta.contains(prevedi(sql_leta.value(sql_leta.record().indexOf("datum_izdaje")).toString()).right(4)) ) {
					leta.append(prevedi(sql_leta.value(sql_leta.record().indexOf("datum_izdaje")).toString()).right(4));
				}
			}

			// razvrsti leta narascajoce
			leta.sort();

			// za vsako leto poisci mesece, dneve in id-je ter jih zapisi v nov seznam
			for ( int i_leta = 0; i_leta < leta.count(); i_leta++ ) {
				// ustvari nov seznam id-jev, mesecev in zaporednih stevilk
				QStringList seznam_vnosov;
				QStringList meseci;
				int zaporedna_stevilka = 0;

				// poisci vse mesece, dobimo seznam mesecev v danem letu
				QSqlQuery sql_meseci;
				sql_meseci.prepare("SELECT * FROM racuni WHERE tip_racuna LIKE '" + pretvori(QString::number(i_tip_racuna, 10)) +
								   "' AND stevilka_racuna NOT LIKE '" +
								   "' AND datum_izdaje LIKE '%." + leta.value(i_leta) + "'");
				sql_meseci.exec();
				while ( sql_meseci.next() ) {
					if ( !meseci.contains(prevedi(sql_meseci.value(sql_meseci.record().indexOf("datum_izdaje")).toString()).right(7).left(2)) ) {
						meseci.append(prevedi(sql_meseci.value(sql_meseci.record().indexOf("datum_izdaje")).toString()).right(7).left(2));
					}
				}

				// razvrsti po mesecu
				meseci.sort();

				// za vsak mesec poiscemo dneve, idje ter jih zapisemo v nov seznam
				for ( int i_meseci = 0; i_meseci < meseci.count(); i_meseci++ ) {

					// ustvarimo nov seznam dnevov
					QStringList dnevi;

					// poisci vse dneve, dobimo seznam dnevov v danem letu in danem mesecu
					QSqlQuery sql_dnevi;
					sql_dnevi.prepare("SELECT * FROM racuni WHERE tip_racuna LIKE '" + pretvori(QString::number(i_tip_racuna, 10)) +
									  "' AND stevilka_racuna NOT LIKE '" +
									  "' AND datum_izdaje LIKE '%." + meseci.value(i_meseci) + "." + leta.value(i_leta) + "'");
					sql_dnevi.exec();
					while ( sql_dnevi.next() ) {
						if ( !dnevi.contains(prevedi(sql_dnevi.value(sql_dnevi.record().indexOf("datum_izdaje")).toString()).left(2)) ) {
							dnevi.append(prevedi(sql_dnevi.value(sql_dnevi.record().indexOf("datum_izdaje")).toString()).left(2));
						}
					}

					// razvrsti po dnevih
					dnevi.sort();

					// za vsak dan poisci id-je ter jih zapisi v nov seznam
					for ( int i_dnevi = 0; i_dnevi < dnevi.count(); i_dnevi++ ) {

						// poisci vse id-je, dobimo seznam id-jev v danem letu in danem mesecu in danem dnevu
						QSqlQuery sql_id_ji;
						sql_id_ji.prepare("SELECT * FROM racuni WHERE tip_racuna LIKE '" + pretvori(QString::number(i_tip_racuna, 10)) +
										  "' AND stevilka_racuna NOT LIKE '" +
										  "' AND datum_izdaje LIKE '" + dnevi.value(i_dnevi) + "." + meseci.value(i_meseci) + "." + leta.value(i_leta) + "'");
						sql_id_ji.exec();
						while ( sql_id_ji.next() ) {
							if ( !seznam_vnosov.contains(prevedi(sql_id_ji.value(sql_id_ji.record().indexOf("id")).toString())) ) {
								seznam_vnosov.append(prevedi(sql_id_ji.value(sql_id_ji.record().indexOf("id")).toString()));
							}
						}

						sql_id_ji.clear();

					} // for ( int i_dnevi = 0; i_dnevi < dnevi.count(); i_dnevi++ )

					dnevi.clear();
					sql_dnevi.clear();

				} // for ( int i_meseci = 0; i_meseci < meseci.count(); i_meseci++ )

				// pojdi cez cel seznam vnosov (notri so IDji po vrstnem redu) in vsakega izpisi ( kasneje popravi )
				for ( int i_seznam_vnosov = 0; i_seznam_vnosov < seznam_vnosov.count(); i_seznam_vnosov++ ) {

					zaporedna_stevilka++;
					i_zaporedna++;

					QString zaporedna = "";
					if ( zaporedna_stevilka < 10 ) {
						zaporedna = "00" + QString::number(zaporedna_stevilka, 10);
					}
					else if ( zaporedna_stevilka < 100 ) {
						zaporedna = "0" + QString::number(zaporedna_stevilka, 10);
					}
					else {
						zaporedna = "" + QString::number(zaporedna_stevilka, 10);
					}

					QString predpona = "";

					if ( i_tip_racuna == 1 ) {
						predpona = "P";
					}
					else if ( i_tip_racuna == 2 ) {
						predpona = "A";
					}
					else if ( i_tip_racuna == 3 ) {
						predpona = "R";
					}
					else if ( i_tip_racuna == 4 ) {
						predpona = "D";
					}

					// imamo dovolj podatkov za tvorbo stevilke racuna

					ui->btn_prestevilci->setText("Shranjujem: " + QString::number(i_zaporedna, 10) + "/" + QString::number(i_vseh_racunov, 10));
					qApp->processEvents();

					QSqlQuery sql_prestevilci;
					sql_prestevilci.prepare("UPDATE racuni SET stevilka_racuna = ? WHERE id LIKE '" + pretvori(seznam_vnosov.value(i_seznam_vnosov)) + "'");
					sql_prestevilci.bindValue(0, pretvori(predpona + leta.value(i_leta).right(2) + zaporedna));
					sql_prestevilci.exec();

				} // for ( int i_seznam_vnosov = 0; i_seznam_vnosov < seznam_vnosov.count(); i_seznam_vnosov++ )

				meseci.clear();
				sql_meseci.clear();
				seznam_vnosov.clear();

			} // for ( int i_leta = 0; i_leta < leta.count(); i_leta++ )

			leta.clear();
			sql_leta.clear();

		} // for ( int i_tip_racuna = 1; i_tip_racuna <= 3; i_tip_racuna++ )

	ui->btn_prestevilci->setText(gumb);
	qApp->processEvents();

	// sporocilo ob zakljucku prestevilcevanja
	QMessageBox zakljucek;
	zakljucek.setText("Prestevilcenje zakljuceno");
	zakljucek.setInformativeText("Prestevilcenje zahtevanih vnosov je bilo uspesno zakljuceno.");
	zakljucek.exec();

	napolni();

}
