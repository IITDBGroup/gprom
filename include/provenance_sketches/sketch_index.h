/*
 *------------------------------------------------------------------------------
 *
 * sketch_index.h - An index for provenance sketches
 *
 *     An index for provenance sketches and methods for persisting it in the
 *     database.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-04-03
 *        SUBDIR: include/provenance_sketches/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _SKETCH_INDEX_H_
#define _SKETCH_INDEX_H_

extern void loadSketchIndexFromDB(char *tablename);
extern void storeSketchIndexToDB(char *tablename);


#endif /* _SKETCH_INDEX_H_ */
