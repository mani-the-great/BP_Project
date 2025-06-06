#include <curses.h>
#include "sqlite3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int ctask,fctask=1;
int csubtask,fcsubtask=1;
int ccat,fccat=1; //first-time current category
int m=0;
int ctaskdone,csubtaskdone;
int ow=0; //open window
static int row=1;
static int subrow=1;
static int catrow=1;
static int task_callback(void *data, int argc, char **argv, char **azColName){
    int num=atoi(argv[0]);
      if(fctask==1){
          fctask=0;
          ctask=num;
      }
      if(ctask==num){
      wattron(data,A_REVERSE);
      ctaskdone=atoi(argv[3]);
      }
      mvwprintw(data,row,2,"%s. ", argv[4]);
      if(*argv[3]=='1')
        wprintw(data, "[X] %s", argv[1]);
      else
        wprintw(data, "[ ] %s", argv[1]);
      row++;
      if(ctask==num)
      wattroff(data,A_REVERSE);
	return 0;
}

static int subtask_callback(void *data, int argc, char **argv, char **azColName){
      int num=atoi(argv[0]);
      if(fcsubtask==1){
          fcsubtask=0;
          csubtask=num;
      }
      if(csubtask==num){
      wattron(data,A_REVERSE);
      csubtaskdone=atoi(argv[3]);
      }
      mvwprintw(data,subrow,2,"%d. ", subrow);
      if(*argv[3]=='1')
        wprintw(data, "[X] %s", argv[2]);
      else
        wprintw(data, "[ ] %s", argv[2]);
      subrow++;
       if(csubtask==num)
      wattroff(data,A_REVERSE);
	return 0;
}

static int des_callback(void *data, int argc, char **argv, char **azColName){
      mvwprintw(data,1,2,"%s", argv[0]);
	return 0;
}

static int cat_callback(void *data, int argc, char **argv, char **azColName){
      int num=atoi(argv[0]);
      if(fccat==1){
          fccat=0;
          ccat=num;
      }
      if(ccat==num)
      wattron(data,A_REVERSE);
      mvwprintw(data,catrow,2,"%s", argv[2]);
      catrow++;
      if(ccat==num)
      wattroff(data,A_REVERSE);
	return 0;
}

static int deadline_callback(void *data, int argc, char **argv, char **azColName){
      mvwprintw(data,1,2,"%s", argv[0]);
	return 0;
}

int main(){
    initscr();
    curs_set(0);
    noecho();
    sqlite3 *db;
	sqlite3_open("database.db", &db);
    const char* data = "Callback function called";
    char *sql;
    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);
    char x;
do{
    WINDOW *tasks=newwin(ymax*0.65,xmax/2,0,0);
    WINDOW *subtasks=newwin(ymax*0.35,xmax/2,ymax*0.65,0);
     WINDOW *description=newwin(ymax*0.4,xmax/2,0,xmax/2);
     WINDOW *categories=newwin(ymax*0.25,xmax/2,ymax*0.4,xmax/2);
     WINDOW *deadline=newwin(ymax*0.35,xmax/2,ymax*0.65,xmax/2);
     char buffer[250];
    void dowindows(){
    // tasks
    
	box(tasks,0,0);
    mvwprintw(tasks,0,2,"Tasks");
    if(m==0)
    sql="SELECT * FROM tasks ORDER BY priority";
    else if(m==1)
    sql="SELECT * FROM tasks ORDER BY title";
    else
    sql="SELECT * FROM tasks ORDER BY deadline";
     sqlite3_exec(db, sql, task_callback, tasks,NULL);
     refresh();
     wrefresh(tasks);
    // subtasks
    
	box(subtasks,0,0);
    mvwprintw(subtasks,0,2,"Subtasks");
    
    sprintf(buffer, "%s%d", "SELECT * FROM subtasks WHERE task_id=", ctask);
    sqlite3_exec(db, buffer, subtask_callback, subtasks,NULL);
	refresh();
	wrefresh(subtasks);
    // description
    //WINDOW *description=newwin(ymax*0.4,xmax/2,0,xmax/2);
	box(description,0,0);
    mvwprintw(description,0,2,"Description");
    sprintf(buffer, "%s%d", "SELECT description FROM tasks WHERE id=", ctask);
    sqlite3_exec(db, buffer, des_callback, description,NULL);
	refresh();
	wrefresh(description);
    // categories
    
	box(categories,0,0);
    mvwprintw(categories,0,2,"Categories");
    sprintf(buffer, "%s%d", "SELECT * FROM categories WHERE task_id=", ctask);
    sqlite3_exec(db, buffer, cat_callback, categories,NULL);
	refresh();
	wrefresh(categories);
    // deadline
    
	box(deadline,0,0);
    mvwprintw(deadline,0,2,"Deadline");
    sprintf(buffer, "%s%d", "SELECT deadline FROM tasks WHERE id=", ctask);
    sqlite3_exec(db, buffer, deadline_callback, deadline,NULL);
	refresh();
	wrefresh(deadline);
    }
    dowindows();
    // edit title of task
    if(ow==1){
       WINDOW *etitletask=newwin(ymax*0.4,xmax/2,ymax*0.3,xmax/4);
	        box(etitletask,0,0);
            mvwprintw(etitletask, 0, 1, "Editing the task");
            mvwprintw(etitletask,1,2,"Enter the new title for this task: ");
            curs_set(1);
            echo();
            char *input;
            wgetstr(etitletask,input);
            sprintf(buffer, "%s%s%s%d", "UPDATE tasks SET title='", input,"' WHERE id=", ctask);
            sqlite3_exec(db, buffer, NULL, NULL, NULL);
            noecho();
            curs_set(0);
            wclear(etitletask);
            delwin(etitletask);
            refresh();
            wrefresh(etitletask);
            ow=0;
            row=1;
            subrow=1;
            catrow=1;
            dowindows();
    }
    // edit desc of task
    if(ow==2){
        WINDOW *edesctask=newwin(ymax*0.4,xmax/2,ymax*0.3,xmax/4);
	        box(edesctask,0,0);
            mvwprintw(edesctask, 0, 1, "Editing the task");
            mvwprintw(edesctask,1,2,"Enter the new description for this task: ");
            curs_set(1);
            echo();
            char *input;
            wgetstr(edesctask,input);
            sprintf(buffer, "%s%s%s%d", "UPDATE tasks SET description='", input,"' WHERE id=", ctask);
            sqlite3_exec(db, buffer, NULL, NULL, NULL);
            noecho();
            curs_set(0);
            wclear(edesctask);
            delwin(edesctask);
            refresh();
            wrefresh(edesctask);
            ow=0;
            row=1;
            subrow=1;
            catrow=1;
            dowindows();
    }
    // edit deadline of task
    if(ow==3){
        WINDOW *edeadtask=newwin(ymax*0.4,xmax/2,ymax*0.3,xmax/4);
	        box(edeadtask,0,0);
            mvwprintw(edeadtask, 0, 1, "Editing the task");
            mvwprintw(edeadtask,1,2,"Enter the new deadline for this task: ");
            curs_set(1);
            echo();
            char *input;
            wgetstr(edeadtask,input);
            sprintf(buffer, "%s%s%s%d", "UPDATE tasks SET deadline='", input,"' WHERE id=", ctask);
            sqlite3_exec(db, buffer, NULL, NULL, NULL);
            noecho();
            curs_set(0);
            wclear(edeadtask);
            delwin(edeadtask);
            refresh();
            wrefresh(edeadtask);
            ow=0;
            row=1;
            subrow=1;
            catrow=1;
            dowindows();
    }
    // add task
    if(ow==4){
            WINDOW *etitletask=newwin(ymax*0.4,xmax/2,ymax*0.3,xmax/4);
	        box(etitletask,0,0);
            mvwprintw(etitletask, 0, 1, "Adding new task");
            mvwprintw(etitletask,1,2,"Enter the title for this new task: ");
            curs_set(1);
            echo();
            char title[200];
            wrefresh(etitletask);
            wgetstr(etitletask,title);
            noecho();
            curs_set(0);
            wclear(etitletask);
            delwin(etitletask);
            refresh();
            wrefresh(etitletask);
            ow=0;
            row=1;
            subrow=1;
            catrow=1;
            dowindows();

            WINDOW *edesctask=newwin(ymax*0.4,xmax/2,ymax*0.3,xmax/4);
	        box(edesctask,0,0);
            mvwprintw(edesctask, 0, 1, "Adding new task");
            mvwprintw(edesctask,1,2,"Enter the description for this new task: ");
            curs_set(1);
            echo();
            char desc[200];
            wrefresh(edesctask);
            wgetstr(edesctask,desc);
            noecho();
            curs_set(0);
            wclear(edesctask);
            delwin(edesctask);
            refresh();
            wrefresh(edesctask);
            ow=0;
            row=1;
            subrow=1;
            catrow=1;
            dowindows();

            char dead[200];
            do{
            WINDOW *edeadtask=newwin(ymax*0.4,xmax/2,ymax*0.3,xmax/4);
            box(edeadtask,0,0);
            mvwprintw(edeadtask, 0, 1, "Adding new task");
            mvwprintw(edeadtask,1,2,"Enter the deadline for this new task: ");
            curs_set(1);
            echo();
            //char dead[200];
            wrefresh(edeadtask);
            wgetstr(edeadtask,dead);
            noecho();
            curs_set(0);
            wclear(edeadtask);
            delwin(edeadtask);
            refresh();
            wrefresh(edeadtask);
            ow=0;
            row=1;
            subrow=1;
            catrow=1;
            dowindows();
            }while(dead[4]!='/' || dead[7]!='/');

            WINDOW *eprioritytask=newwin(ymax*0.4,xmax/2,ymax*0.3,xmax/4);
            box(eprioritytask,0,0);
            mvwprintw(eprioritytask, 0, 1, "Adding new task");
            mvwprintw(eprioritytask,1,2,"Enter the priority for this new task: ");
            curs_set(1);
            echo();
            char priority[200];
            wrefresh(eprioritytask);
            wgetstr(eprioritytask,priority);
            noecho();
            curs_set(0);
            wclear(eprioritytask);
            delwin(eprioritytask);
            refresh();
            wrefresh(eprioritytask);
            ow=0;
            row=1;
            subrow=1;
            catrow=1;
            dowindows();
            sprintf(buffer, "%s%s%s%s%s%s%s%s%s", "INSERT INTO tasks (title,description,done,priority,deadline)VALUES('", title,"','",desc,"',0,",priority,",'",dead,"')");
            sqlite3_exec(db, buffer, NULL, NULL, NULL);
            ow=0;
            row=1;
            subrow=1;
            catrow=1;
            dowindows();
    }
        x=getch();
        switch (x){
            case 'j':
            ctask++;
            break;
            case 'k':
            ctask--;
            break;
            //subtasks
            case 'l':
            char y;
            do{
            y=getch();
                switch (y)
                {
                case ' ':
                 if(csubtaskdone==1){
                sprintf(buffer,"%s%d", "UPDATE subtasks SET done=0 WHERE id=", csubtask);
                sqlite3_exec(db,buffer,NULL,NULL, NULL);
                }else{
                sprintf(buffer,"%s%d", "UPDATE subtasks SET done=1 WHERE id=", csubtask);
                sqlite3_exec(db,buffer,NULL,NULL, NULL); 
                }
                subrow=1;
                wclear(subtasks);
                break;
                case 'j':
                csubtask++;
                subrow=1;
                break;
                case 'k':
                csubtask--;
                subrow=1;
                break;
                case 'a':
            WINDOW *asubtask=newwin(ymax*0.4,xmax/2,ymax*0.3,xmax/4); // add subtask
	        box(asubtask,0,0);
            mvwprintw(asubtask, 0, 1, "Adding a subtask");
            mvwprintw(asubtask,1,2,"Enter the title for this new subtask: ");
            curs_set(1);
            echo();
            char input[200];
            wrefresh(asubtask);
            wgetstr(asubtask,input);
            sprintf(buffer, "%s%d%s%s%s", "INSERT INTO subtasks(task_id,title,done)VALUES(",ctask,", '", input,"',0)");
            sqlite3_exec(db, buffer, NULL, NULL, NULL);
            noecho();
            curs_set(0);
            wclear(asubtask);
            delwin(asubtask);
            refresh();
            wrefresh(asubtask);
            row=1;
            subrow=1;
            catrow=1;
            wclear(subtasks);
            dowindows();
                break;
                case 'd':
                sprintf(buffer,"%s%d", "DELETE FROM subtasks WHERE id=", csubtask);
                sqlite3_exec(db,buffer,NULL,NULL, NULL);
                subrow=1;
                csubtask--;
                wclear(subtasks);
                break;
                default:
                    break;
                }
                subrow=1;
	            box(subtasks,0,0);
                mvwprintw(subtasks,0,2,"Subtasks");
                sprintf(buffer, "%s%d", "SELECT * FROM subtasks WHERE task_id=", ctask);
                sqlite3_exec(db, buffer, subtask_callback, subtasks,NULL);
	            refresh();
	            wrefresh(subtasks);
            }while(y!='h');
            break;
            case 'a':
            ow=4;
            break;
            case 'd':
            sprintf(buffer,"%s%d", "DELETE FROM tasks WHERE id=", ctask);
            sqlite3_exec(db,buffer,NULL,NULL, NULL);
            break;
            case 'e':
            ow=1;
            break;
            case 'r':
            ow=2;
            break;
            case 'n':
            ow=3;
            break;
            //categories
            case 'c':
            char z;
            do{
            z=getch();
            switch (z)
            {
            case 'a':
            WINDOW *acat=newwin(ymax*0.4,xmax/2,ymax*0.3,xmax/4); // add category
	        box(acat,0,0);
            mvwprintw(acat, 0, 1, "Adding a category");
            mvwprintw(acat,1,2,"Enter the title for this new category: ");
            curs_set(1);
            echo();
            char input[200];
            wrefresh(acat);
            wgetstr(acat,input);
            sprintf(buffer, "%s%d%s%s%s", "INSERT INTO categories(task_id,title)VALUES(",ctask,", '", input,"')");
            sqlite3_exec(db, buffer, NULL, NULL, NULL);
            noecho();
            curs_set(0);
            wclear(acat);
            delwin(acat);
            refresh();
            wrefresh(acat);
            catrow=1;
            subrow=1;
            row=1;
            wclear(acat);
            dowindows();
            break;
            case 'd':
            sprintf(buffer,"%s%d", "DELETE FROM categories WHERE id=", ccat);
            sqlite3_exec(db,buffer,NULL,NULL, NULL);
            catrow=1;
            ccat--;
            wclear(categories);
            break;
            case 'j':
            ccat++;
            catrow=1;
            box(categories,0,0);
            break;
            case 'k':
            ccat--;
            catrow=1;
            box(categories,0,0);
            break;
            default:
                break;
            }
            catrow=1;
            box(categories,0,0);
            mvwprintw(categories,0,2,"Categories");
            sprintf(buffer, "%s%d", "SELECT * FROM categories WHERE task_id=", ctask);
            sqlite3_exec(db, buffer, cat_callback, categories,NULL);
	        refresh();
	        wrefresh(categories);
            }while (z!='c');
            
            break;
            case ' ':
            if(ctaskdone==1){
                sprintf(buffer,"%s%d", "UPDATE tasks SET done=0 WHERE id=", ctask);
                sqlite3_exec(db,buffer,NULL,NULL, NULL);
            }else{
                sprintf(buffer,"%s%d", "UPDATE tasks SET done=1 WHERE id=", ctask);
                sqlite3_exec(db,buffer,NULL,NULL, NULL); 
            }
            break;
            case 'm':
            m=1;
            break;
            case 'p':
            m=2;
            break;
            case 'b':
            m=0;
            break;
            default:
            break;
        }
        row=1;
        subrow=1;
        catrow=1;
    }while(x!='q');
    sqlite3_close(db);
    endwin();
}