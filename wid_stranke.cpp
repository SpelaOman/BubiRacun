#include <QtSql>
#include <QMessageBox>
#include <QClipboard>
#include <QTextEdit>

#include "wid_stranke.h"
#include "ui_wid_stranke.h"
#include "stranke.h"
#include "kodiranje.h"

wid_stranke::wid_stranke(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::wid_stranke)
{
	ui->setupUi(this);

	napolni();

}

wid_stranke::~wid_stranke()
{
	delete ui;
}

void wid_stranke::on_cb_fizicna_toggled() {

	napolni();

}

void wid_stranke::on_cb_pravna_toggled() {

	napolni();

}

void wid_stranke::napolni() {

	int izbranec = 0;
	int razvrsti = 0;

	if ( ui->tbl_stranke->selectedItems().count() > 0 ) {
		izbranec = ui->tbl_stranke->selectedItems().takeAt(0)->row();
	}

	razvrsti = ui->tbl_stranke->horizontalHeader()->sortIndicatorSection();

	QString stavek = "SELECT * FROM stranke";

	if ( ! ( ui->cb_fizicna->isChecked() && ui->cb_pravna->isChecked() ) ) {
		if ( ui->cb_fizicna->isChecked() ) {
			stavek += " WHERE tip LIKE '1'";
		}
		if ( ui->cb_pravna->isChecked() ) {
			stavek += " WHERE tip LIKE '2'";
		}
	}

		// clear previous content
		ui->tbl_stranke->clear();

		for (int i = 0; i <= 7; i++) {
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
		ui->tbl_stranke->insertColumn(6);
		ui->tbl_stranke->insertColumn(7);

		QTableWidgetItem *naslov0 = new QTableWidgetItem;
		QTableWidgetItem *naslov1 = new QTableWidgetItem;
		QTableWidgetItem *naslov2 = new QTableWidgetItem;
		QTableWidgetItem *naslov3 = new QTableWidgetItem;
		QTableWidgetItem *naslov4 = new QTableWidgetItem;
		QTableWidgetItem *naslov5 = new QTableWidgetItem;
		QTableWidgetItem *naslov6 = new QTableWidgetItem;
		QTableWidgetItem *naslov7 = new QTableWidgetItem;

		naslov0->setText("ID");
		naslov1->setText("Ime/Naziv");
		naslov2->setText("Priimek/Polni naziv");
		naslov3->setText("Telefon");
		naslov4->setText("GSM");
		naslov5->setText("Elektronski naslov");
		naslov6->setText("Izobr. ustanova");
		naslov7->setText("Fizicna/Pravna");

		ui->tbl_stranke->setHorizontalHeaderItem(0, naslov0);
		ui->tbl_stranke->setHorizontalHeaderItem(1, naslov1);
		ui->tbl_stranke->setHorizontalHeaderItem(2, naslov2);
		ui->tbl_stranke->setHorizontalHeaderItem(3, naslov3);
		ui->tbl_stranke->setHorizontalHeaderItem(4, naslov4);
		ui->tbl_stranke->setHorizontalHeaderItem(5, naslov5);
		ui->tbl_stranke->setHorizontalHeaderItem(6, naslov6);
		ui->tbl_stranke->setHorizontalHeaderItem(7, naslov7);

		ui->tbl_stranke->setColumnWidth(0, 35);
		ui->tbl_stranke->setColumnWidth(1, 80);
		ui->tbl_stranke->setColumnWidth(2, 120);
		ui->tbl_stranke->setColumnWidth(3, 140);
		ui->tbl_stranke->setColumnWidth(4, 140);
		ui->tbl_stranke->setColumnWidth(5, 150);
		ui->tbl_stranke->setColumnWidth(6, 100);
		ui->tbl_stranke->setColumnWidth(7, 100);

		QSqlQuery sql_fill;
		sql_fill.prepare(stavek);
		sql_fill.exec();

		int row = 0;
		while (sql_fill.next()) {
			ui->tbl_stranke->insertRow(row);
			ui->tbl_stranke->setRowHeight(row, 20);
			int col = 0;
			int i = 0;
			QString polja[8] = {"id", "ime", "priimek", "telefon", "gsm", "email", "ustanova", "tip"};

			while (col <= 7) {

				QTableWidgetItem *celica = new QTableWidgetItem;

				// uredi sifrante
				if ( polja[i] == "id" ) {
					celica->setData(Qt::DisplayRole, prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()).toInt());
				}
				else if ( polja[i] == "tip" ) {
					if ( prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()) == "1" ) {
						celica->setText("Fizicna");
					}
					else if ( prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()) == "2" ) {
						celica->setText("Pravna");
					}
					else {
						celica->setText(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()));
					}
				}
				else {
					celica->setText(prevedi(sql_fill.value(sql_fill.record().indexOf(polja[i])).toString()));
				}

				ui->tbl_stranke->setItem(row, col, celica);

				col++;
				i++;

			}

			row++;

		}

	ui->tbl_stranke->selectRow(izbranec);
	ui->tbl_stranke->sortByColumn(razvrsti, Qt::AscendingOrder);

}

void wid_stranke::on_tbl_stranke_doubleClicked() {

	stranke *uredi = new stranke;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos(ui->tbl_stranke->selectedItems().takeAt(0)->text());
	this->disconnect();

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_stranke::on_btn_brisi_clicked() {

	QString id = ui->tbl_stranke->selectedItems().takeAt(0)->text();

		// ob brisanju stranke najprej preverimo, ali ima le-ta projekt ali racun
		bool projekt = false;
		bool racun = false;
		bool nadaljuj = true;

		QSqlQuery sql_preveri;

		// preveri obstoj projekta
		sql_preveri.prepare("SELECT * FROM projekti WHERE stranka LIKE '" + id + "'");
		sql_preveri.exec();
		if ( sql_preveri.next() ) {
			projekt = true;
		}
		sql_preveri.clear();

		// preveri obstoj racuna
		sql_preveri.prepare("SELECT * FROM racuni WHERE stranka LIKE '" + id + "'");
		sql_preveri.exec();
		if ( sql_preveri.next() ) {
			racun = true;
		}

		if ( projekt == true || racun == true ) {
			QString stavek = "Stranka ima odprt";
			if ( projekt == true && racun == true ) {
				stavek.append("a tako projekt kot racun");
			}
			else if ( projekt == true ) {
				stavek.append(" projekt");
			}
			else if ( racun == true ) {
				stavek.append(" racun");
			}
			stavek.append(", zato brisanje ni mozno!");

			QMessageBox msgbox;
			msgbox.setText("Brisanje stranke ni mozno");
			msgbox.setInformativeText(stavek);
			msgbox.exec();

			nadaljuj = false;
		}

		if ( nadaljuj == true )  {
			QSqlQuery sql_brisi;

			// poisci vse projekte, ki jih izbrana stranka ima
			QSqlQuery sql_projekt;
			sql_projekt.prepare("SELECT * FROM projekti WHERE stranka LIKE '" + id + "'");
			sql_projekt.exec();
			while ( sql_projekt.next() ) {

				QString stevilka_projekta = sql_projekt.value(sql_projekt.record().indexOf("id")).toString();

				// izbrisi opravila
				sql_brisi.prepare("DELETE FROM opravila WHERE stevilka_projekta LIKE '" + stevilka_projekta + "'");
				sql_brisi.exec();
				sql_brisi.clear();

				// izbrisi izdane racune
				sql_brisi.prepare("DELETE FROM racuni WHERE projekt LIKE '" + stevilka_projekta + "'");
				sql_brisi.exec();
				sql_brisi.clear();

				// izbrisi zapise
				sql_brisi.prepare("DELETE FROM opombe WHERE stevilka_projekta LIKE '" + stevilka_projekta + "'");
				sql_brisi.exec();
				sql_brisi.clear();

				// izbrisi potne naloge
				QSqlQuery sql_potni_nalog;
				sql_potni_nalog.prepare("SELECT * FROM potni_nalogi WHERE stevilka_projekta LIKE '" + stevilka_projekta + "'");
				sql_potni_nalog.exec();
				while ( sql_potni_nalog.next() ) {
					// izbrisi pot
					QString st_naloga = sql_potni_nalog.value(sql_potni_nalog.record().indexOf("stevilka_naloga")).toString();
					sql_brisi.prepare("DELETE FROM potovanja WHERE potni_nalog LIKE '" + st_naloga + "'");
					sql_brisi.exec();
					sql_brisi.clear();
				}
				sql_brisi.prepare("DELETE FROM potni_nalogi WHERE stevilka_projekta LIKE '" + stevilka_projekta + "'");
				sql_brisi.exec();
				sql_brisi.clear();

				// izbrisi prejete racune
				sql_brisi.prepare("DELETE FROM prejeti_racuni WHERE stevilka_projekta LIKE '" + stevilka_projekta + "'");
				sql_brisi.exec();
				sql_brisi.clear();

				// izbrisi projekt
				sql_brisi.prepare("DELETE FROM projekti WHERE id LIKE '" + stevilka_projekta + "'");
				sql_brisi.exec();

			}

			// izbrisi stranko
			sql_brisi.prepare("DELETE FROM stranke WHERE id LIKE '" + id + "'");
			sql_brisi.exec();

			// odstrani vrstico v tabeli
			ui->tbl_stranke->removeRow(ui->tbl_stranke->selectedItems().takeAt(0)->row());
		}

	osvezi("stranke");

}

void wid_stranke::on_btn_nov_clicked() {

	stranke *uredi = new stranke;
	uredi->show();
	QObject::connect(this, SIGNAL(prenos(QString)),
			   uredi , SLOT(prejem(QString)));
	prenos("Nova stranka");
	this->disconnect();

	// receive signal to refresh table
	QObject::connect(uredi, SIGNAL(poslji(QString)),
			   this , SLOT(osvezi(QString)));

}

void wid_stranke::osvezi(QString beseda) {

	if ( beseda == "stranke" ) {
		napolni();
	}

}

QString wid_stranke::pretvori(QString besedilo) {

	return kodiranje().zakodiraj(besedilo);

}

QString wid_stranke::prevedi(QString besedilo) {

	return kodiranje().odkodiraj(besedilo);

}

void wid_stranke::on_btn_kopiraj_clicked() {

	QClipboard *odlozisce = QApplication::clipboard();

	QModelIndexList selectedList = ui->tbl_stranke->selectionModel()->selectedRows();

	QString html_besedilo = "<table>";
	html_besedilo += "<tr>";
	html_besedilo += "<th>ID</th>";
	html_besedilo += "<th>Ime/Naziv</th>";
	html_besedilo += "<th>Priimek/Polni naziv</th>";
	html_besedilo += "<th>Telefon</th>";
	html_besedilo += "<th>GSM</th>";
	html_besedilo += "<th>Elektronski naslov</th>";
	html_besedilo += "<th>Izobrazevalna ustanova</th>";
	html_besedilo += "<th>Tip stranke</th>";
	html_besedilo += "</tr>";

	for( int i = 0; i < selectedList.count(); i++) {
		html_besedilo += "<tr>";
		for ( int a = 0; a < 8; a++ ) {
			html_besedilo += "<td>";
			html_besedilo += ui->tbl_stranke->item(selectedList.at(i).row(), a)->text();
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
