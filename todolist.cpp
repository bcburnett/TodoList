#include "todolist.h"
#include "ui_todolist.h"

// qmap to hold the game images
QMap<int, QPixmap> TodoList::imap;
// game state
state TodoList::mstate;
// phrase database
QSqlDatabase TodoList::db;
// constructor
TodoList::TodoList(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::TodoList) {
  ui->setupUi(this);

  // open the sqlite database
  TodoList::db = QSqlDatabase::addDatabase("QSQLITE");
  // on my system i have a dir called data in my home folder
  TodoList::db.setDatabaseName(QString(getenv("HOME")) + "/data/letterguess/phrases.db");
  // and finally open the database
  TodoList::db.open();

  // load in the images
  imap = setImages();

  // set the on-screen keyboard / used letter display to call the keyboard button pressed function for all buttons except the reset button.
  for (int i = 0; i < ui->keyboardLayout->count(); ++i) {
    QWidget* widget = ui->keyboardLayout->itemAt( i )->widget();
    QPushButton* button = qobject_cast<QPushButton*>( widget );
    if (button  && !(button->text() == "RESET")) connect( button, SIGNAL(clicked()), this, SLOT(keyboardButtonPressed()) );
  };

  // call the reset function to start a new game
  on_RESET_clicked();
}

// destructor
TodoList::~TodoList() {
  db.close();
  delete ui;
}

//slots
void TodoList::keyboardButtonPressed() {
  QPushButton* button = qobject_cast<QPushButton*>( sender() );
  // button holds the button that was pressed

  // if a button was clicked and it wasn't the reset button
  if ( button && !(button->text() == "RESET")  ) {
    //disable the button
    button->setEnabled(false);
    // send the letter from the button text and the current state to the checkletter function
    TodoList::mstate = checkletter(button->text(), TodoList::mstate);
    //pass the state to the display function to update the display
    setDisplay(TodoList::mstate);
  }
}

void TodoList::on_RESET_clicked() {
  // set the base game state with a new phrase and category
  TodoList::mstate = clearstate(TodoList::mstate);
  // if something went wrong with the database (there may be a table or two without a line #1) try again
  if(TodoList::mstate.phrase == "") on_RESET_clicked();
  // pass the state to the display function
  setDisplay(TodoList::mstate);
}

//functions
// sets all letter buttons to enabled or disabled based on action being true or false
void TodoList::enabledisableKeyboard(bool action) {
  for (int i = 0; i < ui->keyboardLayout->count(); ++i) {
    // get a button
    QPushButton* button = qobject_cast<QPushButton*>( ui->keyboardLayout->itemAt( i )->widget());
    // set its enabled property to action unless it is the reset button
    button->setEnabled(action || (button->text() == "RESET"));
  };
}

void TodoList::setDisplay(state mstate) {
  // the image to display happy to sad emoticons or win/lose images
  ui->image->setPixmap(imap[mstate.wrongguesses]);
  // the category from the database
  ui->category->setText(mstate.category);
  // the semi-revealed phrase
  ui->phrase->setText(mstate.displayedphrase);
}

// load the images into the image map
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
  // reset the state to "zero"
  mstate.displayedphrase.clear();
  mstate.gameover = false;
  mstate.wrongguesses = 0;
  mstate.phrase.clear();
  mstate.category.clear();
  mstate.parsedphrase.clear();

  // query the database for the category and phrase
  // first pick a random category, then pick a random phrase from that table
  QSqlQuery query;
  query.prepare("SELECT tablename, title FROM categories limit 1 offset abs(random()) % (select count(*)from categories)");
  if(query.exec() && query.first()) {
    auto table = query.value(0).toString();
    auto phrase = "SELECT phrase FROM " + table + " limit 1 offset abs(random()) % (select count(*)from " + table + ")";
    QSqlQuery queryphrase;
    queryphrase.prepare(phrase);
    if(queryphrase.exec() && queryphrase.first()) {
      // set the state variables for phrase and category
      mstate.phrase = queryphrase.value(0).toString();
      mstate.category = query.value(1).toString();
    }
  }

  for(int i = 0; i < mstate.phrase.length(); i++ ) {
    // parse the phrase and generate the displayed phrase for the user
    mstate.parsedphrase[i].chr = mstate.phrase.at(i);
    mstate.parsedphrase[i].show = QString("- '?&\"").contains(mstate.phrase.at(i));
    mstate.displayedphrase += (mstate.parsedphrase[i].show ? mstate.parsedphrase[i].chr : "_");
  }

  // enable the keyboard
  enabledisableKeyboard(true);
  return mstate;
}

state TodoList::checkletter(QString letter, state mstate) {
  // clear the displayed phrase
  mstate.displayedphrase.clear();
  // if the user entered a correct letter, right will be true after the next check
  bool right = false;
  foreach(int i, mstate.parsedphrase.keys()) if(mstate.parsedphrase[i].chr == letter) right = mstate.parsedphrase[i].show = true ;
  // increment wrongguesses by 0 or 1 depending on wether they guessed a correct letter
  mstate.wrongguesses += !right;
  //generate the new displayed phrase
  foreach(mychr i, mstate.parsedphrase.values()) mstate.displayedphrase += i.show ? i.chr : "_";
  // check if the game is won
  mstate.gameover = true;
  // if all the letters are guessed, gameover will still be true after this check
  foreach(mychr i, mstate.parsedphrase.values()) mstate.gameover &= i.show;
  if(mstate.gameover) return gameWon(mstate);
  // check if the game is lost
  if(mstate.wrongguesses > 6) return gameLost(mstate);
  // otherwise pass back current state
  return mstate;
}

// set the winning state
state TodoList::gameWon(state mstate) {
  // disable the keyboard except for the reset key
  enabledisableKeyboard(false);
  mstate.wrongguesses = 7;
  mstate.category += "   Congratulations You Won!";
  return mstate;
}

// set the loosing state
state TodoList::gameLost(state mstate) {
  // disable the keyboard except for the reset key
  enabledisableKeyboard(false);
  mstate.gameover = true;
  mstate.wrongguesses = 8;
  mstate.category += "   Sorry, You Lost";
  mstate.displayedphrase = mstate.phrase;
  return mstate;
}


