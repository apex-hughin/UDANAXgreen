/* Copyright � 1979-1999 Udanax.com. All rights reserved.

* This code is licensed under the terms of The Udanax Open-Source License, 
* which contains precisely the terms of the X11 License.  The full text of 
* The Udanax Open-Source License can be found in the distribution in the file 
* license.html.  If this file is absent, a copy can be found at 
* http://udanax.xanadu.com/license.html and http://www.udanax.com/license.html
*/
/***************************************************
  Copyright (c) 1987 Xanadu Operating Company
  XU.87.1 Frontend Source Code:   freealloc.d
***************************************************/

#include "fest.h"
#include "feminreq.h"

/*#define LINKUNUSEDALLOCROUTINES */

nat specslyingaround = 0;
nat spanslyingaround = 0;
nat  charspanslyingaround = 0;
nat documentslyingaround = 0;
nat windowslyingaround = 0;

  nat *
ealloc(i)
  nat i;
{
  nat *falloc();
  nat *ret;

        loop {
                if (ret = falloc ((unsigned)i)){
                        return (ret);
                }else{
/*
 fprintf(stderr,"attempting to grimly reap vm\n\r");
*/
                        grimlyreapvm ();
                }
        }
}


  windowtype *
windowalloc ()
{       /* allocates window and first spec and zeros all other ptrs*/
  windowtype *temp;
  spectype *specalloc();
  maskspectype *maskspecalloc();
  med i;
  nat *ealloc();

        temp = (windowtype *) ealloc (sizeof(windowtype));
        temp -> nextwindow = NULL;
        temp -> prevwindow = NULL;
        temp -> superordinatewindow = NULL;
        temp -> subordinatewindow = NULL;

        temp -> windowwidth = screenwidth;
        temp -> windowheight = screenheight-2;
        temp -> lm = 0;
        temp -> tm = 0;
        for (i = 0; i <= screenheight; i++) {
                temp->windowleftmargins[i] = 0;
                temp->windowline_glorpfed[i] = TRUE;
        }

        temp -> windowview.characterdisplacementwithinview = 0;
        temp -> windowview.viewspecptr = specalloc ();
        temp -> windowview.viewmaskspecptr = specalloc();
        temp -> windowview.viewfromset = NULL;
        temp -> windowview.viewtoset = NULL;

        temp -> ispartofparalleltextface = FALSE;
        temp -> correspondingwindow = NULL;
	temp -> dependentscreenp = FALSE;
	temp -> newlycreateddocumentp = FALSE;
        ++windowslyingaround;
        return(temp);
}
  void
windowfree (windowptr)
  windowtype *windowptr;
{
  bool validallocthing();
  void freecurseswindow();

        if (windowptr == &theoneandonlyscreen.basewindow)
                gerror ("Won't let you free basewindow.");
/*        if(!validallocthing(windowptr))
                return;
*/        specfree (windowptr->windowview.viewspecptr);
        specfree (windowptr->windowview.viewmaskspecptr);
        specfree (windowptr->windowview.viewtoset);
        specfree (windowptr->windowview.viewfromset);
	freecurseswindow(windowptr->w);
	if (windowptr->border)
		freecurseswindow(windowptr->border);

        ffree ((char *)windowptr);

        --windowslyingaround;
}

  spantype *
spanalloc ()
{
  spantype *temp;
  nat *ealloc();

        temp = (spantype *) ealloc (sizeof(spantype));
        temp -> typeofthing = SPANTYPE;
        temp -> nextspan = NULL;
        temp -> prevspan = NULL;
        clearvdisplacement (&temp->vstartaddress);
        temp -> sizeofspan = 0;
        temp -> mask = NORMALDISPLAY;
        ++spanslyingaround;
        return (temp);
}

  void
spanfree (spanptr)
  spantype *spanptr;
{
  spantype *next;
  bool validallocthing();


        for (; spanptr; spanptr = next) {
                next = spanptr->nextspan;
                if (--spanslyingaround < 0)
                        gerror ("freeing more spans than were allocated.");
                if (spanptr->typeofthing != SPANTYPE)
                        gerror ("spanfree called with something not allocated as a span.");
                ffree ((char *)spanptr);
        }
}

  spectype *
specalloc ()
{
  spectype *temp;
  nat *ealloc();

        temp = (spectype *) ealloc (sizeof(spectype));
        temp -> typeofthing = SPECTYPE;
        temp -> nextspec = NULL;
        temp -> prevspec = NULL;
        cleardocid (&temp->docid);
        temp -> specspanptr = NULL;
        ++specslyingaround;
        return (temp);
}

  spectype *
specspanalloc ()  /* allocate spec and first span in it both null otherwise*/
{
  spectype *temp, *specalloc();
  spantype *spanalloc(),*ztemp;

        temp = specalloc ();
        appendvmtypethingtolist ( (vmthingtype*)temp, (vmthingtype*)(ztemp=spanalloc ()));

        if(ztemp != temp->specspanptr)
                gerror("boom in specspanalloc");
if (!temp)
gerror ("got fucked up in specspanalloc");
        return (temp);
}

  void
specfree (specptr)
  spectype *specptr;
{
  spectype *next;
  bool validallocthing();

        if(!validallocthinge((char *)specptr))
                return;
        for (; specptr; specptr = next) {
                next = specptr->nextspec;
                spanfree (specptr->specspanptr);
                if (--specslyingaround < 0)
                        gerror ("freeing more specs than were allocated.");
                if (specptr->typeofthing != SPECTYPE){
        dumpspec(specptr);
                        gerror ("specfree called with something not allocated as a spec.");               
                }
                ffree ((char *)specptr);
        }
}


  charspantype *
charspanalloc (numberofcharsinspan)  /* number of chars? */
  med numberofcharsinspan;
{
  charspantype *temp;

        temp = (charspantype *) ealloc (sizeof(charspantype));
        temp -> typeofthing = CHARSPANTYPE;
        temp -> prevcharspan = NULL;
        temp -> nextcharspan = NULL;
        memclear ((char *)&temp->vstartaddress, sizeof(smalltumbler));
        temp -> numberofcharactersinspan = numberofcharsinspan;
        temp -> charinspanptr = (char *)ealloc (numberofcharsinspan);
        ++charspanslyingaround;
        return (temp);
}
  void
charspanfree (charspanptr)
  charspantype *charspanptr;
{
  charspantype *next;

        while (charspanptr) {
                if (charspanptr->typeofthing != CHARSPANTYPE)
                        gerror ("charspanfree called with something not allocated as a charspan.");
                next = charspanptr->nextcharspan;
                if(validallocthing(charspanptr->charinspanptr))
                        ffree (charspanptr->charinspanptr);
                if(validallocthing((char *)charspanptr))
                        ffree ((char *)charspanptr);
                if (--charspanslyingaround < 0)
                        gerror ("freeing more charspans than were allocated.");
                charspanptr = next;
        }
}

  documenttype *
documentalloc ()
{
  documenttype *temp;
  nat *ealloc();

        temp = (documenttype *) ealloc (sizeof(documenttype));
        temp -> typeofthing = DOCUMENTTYPE;
        temp -> nextdocument = NULL;
        temp -> prevdocument = NULL;
        cleardocid (&temp->documentid);
        temp -> doccharspanset = NULL;
	temp->istextsizevalid = FALSE;
        ++documentslyingaround;
        return (temp);
}

  void
documentfree (documentptr)
  documenttype *documentptr;
{
  bool validallocthing();
/*        if(!validallocthing(documentptr))
                return;
*/        if (documentptr->typeofthing != DOCUMENTTYPE)
                gerror ("documentfree called with something not allocated as a document.");
        charspanfree (documentptr->doccharspanset);
        ffree ((char *)documentptr);
        --documentslyingaround;
}

  med *
taskalloc (taskptr, nbytes)
  tasktype *taskptr;
  med nbytes;
{

       return ((med *)talloc (&taskptr->tempspace, nbytes));
}

/* lint, will go crazy */
  char * /* I don't quite understand this code <reg 2-19-86> but it seems to work*/

talloc (spaceptr, nbytes)
  char **spaceptr;
  med nbytes;
{
  char **p;
  nat *ealloc();

        p = (char **)ealloc (sizeof (*p) + nbytes);
        *p = *spaceptr;
        *spaceptr =(char *) p;
        return (char*)(++p);
}

  void
tfree (spaceptr)
  char **spaceptr;
{
  char **p, **q;

        for (p = (char **)*spaceptr; p; p = q) {
                q = (char **)*p;
                if(validallocthing((char *)p))
                        ffree ((char *)p);
        }
        *spaceptr = NULL;
}

  void
freecorrespondencelist(correspondencelist)
  correspondencetype *correspondencelist;
{        
  correspondencetype *temp,*next;
        for(temp = correspondencelist; temp ; temp = next){
                next =  temp->nextcorrespondence;
                ffree((char *)temp);
        }
}               




#ifdef LINKUNUSEDALLOCROUTINES       /* these routines aren't used yet*/

  docrangetype *
docrangealloc ()
{
  docrangetype *temp;
  nat *ealloc();

        temp = (docrangetype *) ealloc (sizeof(docrangetype));
        temp -> typeofthing = DOCRANGETYPE;
        cleardocid (&temp->docrangeorigin);
        cleardocid (&temp->docrangewidth);
        return (temp);
}
  void
docrangefree (docrange)
  docrangetype *docrange;
{
        ffree (docrange);
}

  tumbleritemtype *
tumbleritemalloc ()
{
  tumbleritemtype *temp;
  nat *ealloc();

        temp = (tumbleritemtype *) ealloc (sizeof(tumbleritemtype));
        temp -> typeofthing = TUMBLERITEMTYPE;
        temp -> nexttumbleritem = NULL;
        temp -> prevtumbleritem = NULL;
        memclear (&temp->value, sizeof (temp->value));
        return (temp);
}

  maskspectype *
maskspecalloc ()
{
  spectype *specalloc();

        return ((maskspectype *) specalloc ());
}

  void
displaymaskfree (displaymaskptr)
  masktype *displaymaskptr;
{
/*        ffree (displaymaskptr);*/
}

  void
tumbleralloc(tumblerptr)
  tumbler *tumblerptr;
{
  nat *ealloc();
        return(ealloc(sizeof(tumbler)));
}

  void
tumblerfree (tumblerptr)
  tumbler *tumblerptr;
{
        ffree(tumblerptr);
}


  viewtype *
viewalloc()
{
  viewtype *temp;
  nat *ealloc();

        temp = (viewtype *) ealloc(sizeof(viewtype));
        temp ->characterdisplacementwithinview = 0;
        temp ->viewspecptr = specalloc();
        temp ->viewmaskspecptr =NULL/* maskspecalloc()*/;
        return(temp);
}

#endif
