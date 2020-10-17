#include"functions.h"
#include"todolist.h"
#include<QMap>
#include<QPixmap>
QMap<int, QPixmap> setImages() {
  QMap<int, QPixmap> imap;
  imap[0] = QPixmap(":/img/img/s0.png");
  imap[1] = QPixmap(":/img/img/s1.png");
  imap[2] = QPixmap(":/img/img/s2.png");
  imap[3] = QPixmap(":/img/img/s3.png");
  imap[4] = QPixmap(":/img/img/s4.png");
  imap[5] = QPixmap(":/img/img/s5.png");
  imap[6] = QPixmap(":/img/img/s6.png");
  imap[7] = QPixmap(":/img/img/giphy.gif");
  imap[8] = QPixmap(":/img/img/loose.gif");
  return imap;
}

state checkletter(QString letter, state mstate) {
  mstate.displayedphrase = "";
  bool right = false;
  foreach(int i, mstate.parsedphrase.keys()) {
    if(mstate.parsedphrase[i].chr == letter) right = mstate.parsedphrase[i].show = true ;
    mstate.displayedphrase += (mstate.parsedphrase[i].show ? mstate.parsedphrase[i].chr : "_");
  }

  if(!right) mstate.wrongguesses += 1;

  mstate.gamewon = true;
  foreach(int i, mstate.parsedphrase.keys()) mstate.gamewon = mstate.gamewon && mstate.parsedphrase[i].show;
  if(mstate.gamewon) {
    mstate.wrongguesses = 7;
    mstate.category = "Congratulations You Won!";
  }
  // check if the game is lost
  if(mstate.wrongguesses > 6 && !mstate.gamewon) {
    mstate.gamelost = true;
    mstate.wrongguesses = 8;
    mstate.category = "Sorry, You Lost";
    mstate.displayedphrase = "";
    foreach(int i, mstate.parsedphrase.keys()) mstate.displayedphrase += mstate.parsedphrase[i].chr;
  }
  return mstate;
}

state getcatandphrase(state mstate) {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(QString(getenv("HOME")) + "/data/letterguess/phrases.db");
  db.open();
  QSqlQuery query;
  query.prepare("SELECT tablename, title FROM categories limit 1 offset abs(random()) % (select count(*)from categories)");
  if(query.exec() && query.first()) {
    QSqlQuery queryphrase;
    auto table = query.value(0).toString();
    auto phrase = "SELECT phrase FROM " + table + " limit 1 offset abs(random()) % (select count(*)from " + table + ")";
    queryphrase.prepare(phrase);
    if(queryphrase.exec() && queryphrase.first()) {
      mstate.phrase = queryphrase.value(0).toString();
      mstate.category = query.value(1).toString();
      return mstate;
    } else {
      mstate.dbfail = true;
      return mstate;
    }
  } else {
    exit(1);
  }
}
