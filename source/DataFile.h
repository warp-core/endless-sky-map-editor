/* DataFile.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef DATA_FILE_H_
#define DATA_FILE_H_

#include "DataNode.h"

#include <QString>

#include <list>



// A class which represents a hierarchical data file. Each line of the file that
// is not empty or a comment is a "node," and the relationship between the nodes
// is determined by indentation: if a node is more indented than the node before
// it, it is a "child" of that node. Otherwise, it is a "sibling." Each node is
// just a collection of one or more tokens that can be interpreted either as
// strings or as floating point values; see DataNode for more information.
class DataFile {
public:
    DataFile();
    DataFile(const QString &path);

    void Load(const QString &path);

    std::list<DataNode>::const_iterator begin() const;
    std::list<DataNode>::const_iterator end() const;

    // Get all the comments that were stripped out when reading.
    const QString &Comments() const;


private:
    DataNode root;
    QString comments;
};



#endif
