#include "todolist.h"
#include "ui_todolist.h"

// dictionary to hold the game images
QVector<QPixmap> TodoList::imap;
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
  TodoList::db.setDatabaseName(QString( "/var/lib/letterguess/phrases.db"));
  // and finally open the database
  TodoList::db.open();

  // load in the images
  imap = setImages();

  // set the on-screen keyboard / used letter display to call the keyboard button pressed function for all buttons except the reset button.
  // we need to make that distinction as the reset button is attached to the on_RESET_clicked function, but the for loop
  // will pick it up as a member of the keyboard layout grid layout container. we only need one handler for the letter buttons
  // because we are doing the same thing regardless of which button was pressed.
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
// these functions are connected to ui events

void TodoList::keyboardButtonPressed() {
  // button holds the button that was pressed
  QPushButton* button = qobject_cast<QPushButton*>( sender() );
  // if a button was clicked and it wasn't the reset button
  if ( button && !(button->text() == "RESET")  ) {
    //disable the button
    button->setEnabled(false);
    // send the letter from the button text and the current state to the checkletter function and get next state
    TodoList::mstate = checkletter(button->text(), TodoList::mstate);
    //pass the state to the display function to update the display
    setDisplay(TodoList::mstate);
  }
}

void TodoList::on_RESET_clicked() {
  // set the base game state with a new phrase and category
  TodoList::mstate = clearstate(TodoList::mstate);
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
QVector<QPixmap> TodoList::setImages() {
  QVector<QPixmap> imap;
  imap.append(QPixmap(":/img/img/s0.png"));
  imap.append(QPixmap(":/img/img/s1.png"));
  imap.append(QPixmap(":/img/img/s2.png"));
  imap.append(QPixmap(":/img/img/s3.png"));
  imap.append(QPixmap(":/img/img/s4.png"));
  imap.append(QPixmap(":/img/img/s5.png"));
  imap.append(QPixmap(":/img/img/s6.png"));
  imap.append(QPixmap(":/img/img/giphy.gif"));
  imap.append(QPixmap(":/img/img/loose.gif"));
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
  query.prepare("SELECT * FROM random_category");
  try {
    query.exec();
    query.first();
  } catch(...) {
    QMessageBox msgBox;
    msgBox.setText("Cannot select a category.");
    msgBox.exec();
    exit(1);
  }
  QSqlQuery queryphrase;
  //  deepcode ignore Sqli: the database is a read-only file, no injection possible as there is no way for the user to specify the table name or change the database in any way from within the program.
  queryphrase.prepare("SELECT phrase FROM " +   query.value(0).toString()  + " order by Random() limit 1");
  if(queryphrase.exec() && queryphrase.first()) {
    // set the state variables for phrase and category
    mstate.phrase = queryphrase.value(0).toString();
    mstate.category = query.value(1).toString();
  }
  if(mstate.phrase == "") on_RESET_clicked();


  for(int i = 0; i < mstate.phrase.length(); i++ ) {
    // parse the phrase and generate the displayed phrase for the user
    // assign the character
    mstate.parsedphrase[i].chr = mstate.phrase.at(i);
    // set the visible state of the character based on the 'freeplay' characters
    mstate.parsedphrase[i].show = QString("!- '?&\"").contains(mstate.phrase.at(i));
    // build the displayed phrase string
    mstate.displayedphrase += (mstate.parsedphrase[i].show ? mstate.parsedphrase[i].chr : "_");
  }

  // enable the keyboard
  enabledisableKeyboard(true);
  // put the game in play
  return mstate;
}

state TodoList::checkletter(QString letter, state mstate) {
  // clear the displayed phrase
  mstate.displayedphrase.clear();
  // if the user entered a correct letter, right will be true after the next check
  bool right = false;
  for(int i : mstate.parsedphrase.keys()) if(mstate.parsedphrase[i].chr == letter) right = mstate.parsedphrase[i].show = true ;
  // increment wrongguesses by 0 or 1 depending on wether they guessed a correct letter
  mstate.wrongguesses += !right;
  //generate the new displayed phrase
  for(mychr i : mstate.parsedphrase.values()) mstate.displayedphrase += i.show ? i.chr : "_";
  // check if the game is won
  mstate.gameover = true;
  // if all the letters are guessed, gameover will still be true after this check
  for(mychr i : mstate.parsedphrase.values()) mstate.gameover &= i.show;
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


