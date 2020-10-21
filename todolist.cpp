#include "todolist.h"
#include "ui_todolist.h"

QMap<int, QPixmap> TodoList::imap;
state TodoList::mstate;
QSqlDatabase TodoList::db;
TodoList::TodoList(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::TodoList) {
  ui->setupUi(this);
  TodoList::db = QSqlDatabase::addDatabase("QSQLITE");
  TodoList::db.setDatabaseName(QString(getenv("HOME")) + "/data/letterguess/phrases.db");
  TodoList::db.open();
  imap = setImages();
  for (int i = 0; i < ui->keyboardLayout->count(); ++i) {
    QWidget* widget = ui->keyboardLayout->itemAt( i )->widget();
    QPushButton* button = qobject_cast<QPushButton*>( widget );
    if (button  && !(button->text() == "RESET")) connect( button, SIGNAL(clicked()), this, SLOT(keyboardButtonPressed()) );
  };
  on_RESET_clicked();
}

TodoList::~TodoList() {
  db.close();
  delete ui;
}

//slots
void TodoList::keyboardButtonPressed() {
  QPushButton* button = qobject_cast<QPushButton*>( sender() );

  if ( button && !(button->text() == "RESET")  ) {
    button->setEnabled(false);
    TodoList::mstate = checkletter(button->text(), TodoList::mstate);
    setDisplay(TodoList::mstate);
  }
}

void TodoList::on_RESET_clicked() {
  TodoList::mstate = clearstate(TodoList::mstate);
  if(TodoList::mstate.phrase == "") on_RESET_clicked();
  setDisplay(TodoList::mstate);
}

//functions
void TodoList::enabledisableKeyboard(bool action) {
  for (int i = 0; i < ui->keyboardLayout->count(); ++i) {
    QPushButton* button = qobject_cast<QPushButton*>( ui->keyboardLayout->itemAt( i )->widget());
    button->setEnabled(action || (button->text() == "RESET"));
  };
}

void TodoList::setDisplay(state mstate) {
  ui->image->setPixmap(imap[mstate.wrongguesses]);
  ui->category->setText(mstate.category);
  ui->phrase->setText(mstate.displayedphrase);
}

QMap<int, QPixmap> TodoList::setImages() {
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

state TodoList::clearstate(state mstate) {
  mstate.displayedphrase.clear();
  mstate.gameover = false;
  mstate.wrongguesses = 0;
  mstate.phrase.clear();
  mstate.category.clear();
  mstate.parsedphrase.clear();

  QSqlQuery query;
  query.prepare("SELECT tablename, title FROM categories limit 1 offset abs(random()) % (select count(*)from categories)");
  if(query.exec() && query.first()) {
    auto table = query.value(0).toString();
    auto phrase = "SELECT phrase FROM " + table + " limit 1 offset abs(random()) % (select count(*)from " + table + ")";
    QSqlQuery queryphrase;
    queryphrase.prepare(phrase);
    if(queryphrase.exec() && queryphrase.first()) {
      mstate.phrase = queryphrase.value(0).toString();
      mstate.category = query.value(1).toString();
    }
  }

  for(int i = 0; i < mstate.phrase.length(); i++ ) {
    mstate.parsedphrase[i].chr = mstate.phrase.at(i);
    mstate.parsedphrase[i].show = QString("- '?&\"").contains(mstate.phrase.at(i));
    mstate.displayedphrase += (mstate.parsedphrase[i].show ? mstate.parsedphrase[i].chr : "_");
  }

  enabledisableKeyboard(true);
  return mstate;
}

state TodoList::checkletter(QString letter, state mstate) {
  mstate.displayedphrase.clear();
  bool right = false;
  foreach(int i, mstate.parsedphrase.keys()) if(mstate.parsedphrase[i].chr == letter) right = mstate.parsedphrase[i].show = true ;
  mstate.wrongguesses += !right;
  foreach(mychr i, mstate.parsedphrase.values()) mstate.displayedphrase += i.show ? i.chr : "_";
  mstate.gameover = true;
  foreach(mychr i, mstate.parsedphrase.values()) mstate.gameover &= i.show;
  if(mstate.gameover) return gameWon(mstate);
  if(mstate.wrongguesses > 6) return gameLost(mstate);
  return mstate;
}

state TodoList::gameWon(state mstate) {
  enabledisableKeyboard(false);
  mstate.wrongguesses = 7;
  mstate.category += "   Congratulations You Won!";
  return mstate;
}

state TodoList::gameLost(state mstate) {
  enabledisableKeyboard(false);
  mstate.gameover = true;
  mstate.wrongguesses = 8;
  mstate.category += "   Sorry, You Lost";
  mstate.displayedphrase = mstate.phrase;
  return mstate;
}


