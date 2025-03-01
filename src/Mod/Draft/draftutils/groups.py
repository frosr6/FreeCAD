# -*- coding: utf-8 -*-
# ***************************************************************************
# *   (c) 2009, 2010                                                        *
# *   Yorik van Havre <yorik@uncreated.net>, Ken Cline <cline@frii.com>     *
# *   (c) 2020 Eliud Cabrera Castillo <e.cabrera-castillo@tum.de>           *
# *                                                                         *
# *   This file is part of the FreeCAD CAx development system.              *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU Lesser General Public License (LGPL)    *
# *   as published by the Free Software Foundation; either version 2 of     *
# *   the License, or (at your option) any later version.                   *
# *   for detail see the LICENCE text file.                                 *
# *                                                                         *
# *   FreeCAD is distributed in the hope that it will be useful,            *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU Library General Public License for more details.                  *
# *                                                                         *
# *   You should have received a copy of the GNU Library General Public     *
# *   License along with FreeCAD; if not, write to the Free Software        *
# *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
# *   USA                                                                   *
# *                                                                         *
# ***************************************************************************
"""Provides utility functions to do operations with groups.

The functions here are also used in the Arch Workbench as some of
the objects created with this workbench work like groups.
"""
## @package groups
# \ingroup draftutils
# \brief Provides utility functions to do operations with groups.

## \addtogroup draftutils
# @{
import FreeCAD as App
import draftutils.utils as utils

from draftutils.translate import _tr
from draftutils.messages import _msg, _err


def get_group_names(doc=None):
    """Return a list of names of existing groups in the document.

    Parameters
    ----------
    doc: App::Document, optional
        It defaults to `None`.
        A document on which to search group names.
        It if is `None` it will search the current document.

    Returns
    -------
    list of str
        A list of names of objects that are "groups".
        These are objects derived from `App::DocumentObjectGroup`
        or which are of types `'Floor'`, `'Building'`, or `'Site'`
        from the Arch Workbench.

        Otherwise, return an empty list.
    """
    if not doc:
        found, doc = utils.find_doc(App.activeDocument())

    if not found:
        _err(_tr("No active document. Aborting."))
        return []

    glist = []

    for obj in doc.Objects:
        if (obj.isDerivedFrom("App::DocumentObjectGroup")
                or utils.get_type(obj) in ("Floor", "Building", "Site")):
            glist.append(obj.Name)

    return glist


def getGroupNames():
    """Return a list of group names. DEPRECATED."""
    utils.use_instead("get_group_names")
    return get_group_names()


def ungroup(obj):
    """Remove the object from any group to which it belongs.

    A "group" is any object returned by `get_group_names`.

    Parameters
    ----------
    obj: App::DocumentObject or str
        Any type of object.
        If it is a string, it must be the `Label` of that object.
        Since a label is not guaranteed to be unique in a document,
        it will use the first object found with this label.
    """
    if isinstance(obj, str):
        obj_str = obj

    found, obj = utils.find_object(obj, doc=App.activeDocument())
    if not found:
        _msg("obj: {}".format(obj_str))
        _err(_tr("Wrong input: object not in document."))
        return None

    doc = obj.Document

    for name in get_group_names():
        group = doc.getObject(name)
        if obj in group.Group:
            # The list of objects cannot be modified directly,
            # so a new list is created, this new list is modified,
            # and then it is assigned over the older list.
            objects = group.Group
            objects.remove(obj)
            group.Group = objects


def get_windows(obj):
    """Return the windows and rebars inside a host.

    Parameters
    ----------
    obj: App::DocumentObject
        A scripted object of type `'Wall'` or `'Structure'`
        (Arch Workbench).
        This will be searched for objects of type `'Window'` and `'Rebar'`,
        and clones of them, and the found elements will be added
        to the output list.

        The function will search recursively all elements under `obj.OutList`,
        in case the windows and rebars are nested under other walls
        and structures.

    Returns
    -------
    list
        A list of all found windows and rebars in `obj`.
        If `obj` is itself a `'Window'` or a `'Rebar'`, or a clone of them,
        it will return the same `obj` element.
    """
    out = []
    item = obj

    if isinstance(item, tuple):
        obj = item[0].getSubObject(item[1], retType=1)
        if not obj:
            return out

    # getLinkedObject() will return the obj itself if it is not a link
    linked = obj.getLinkedObject()

    if utils.get_type(linked) in ("Wall", "Structure"):
        if obj is not item:
            for o in obj.OutList:
                for child, sub in get_windows((o, '')):
                    out.append(item[0], item[1] + child.Name + '.' + sub)
        else:
            for o in obj.OutList:
                out.extend(get_windows(o))

            for i in obj.InList:
                ilinked = i.getLinkedObject()
                if (utils.get_type(ilinked) == "Window"
                        or utils.is_clone(ilinked, "Window")):
                    if hasattr(i, "Hosts"):
                        if obj in i.Hosts:
                            out.append(i)
                elif (utils.get_type(ilinked) == "Rebar"
                    or utils.is_clone(ilinked, "Rebar")):
                    if hasattr(i, "Host"):
                        if obj == i.Host:
                            out.append(i)

    elif (utils.get_type(linked) in ("Window", "Rebar")
          or utils.is_clone(linked, ["Window", "Rebar"])):
        out.append(item)

    return out


def get_group_contents(objectslist,
                       walls=False, addgroups=False,
                       spaces=False, noarchchild=False):
    """Return a list of objects from expanding the input groups.

    The function accepts any type of object, although it is most useful
    with "groups", as it is meant to unpack the objects inside these groups.

    Parameters
    ----------
    objectslist: list
        If any object in the list is a group, its contents (`obj.Group`)
        are extracted and added to the output list.

        The "groups" are objects derived from `App::DocumentObjectGroup`,
        but they can also be `'App::Part'`, or `'Building'`, `'BuildingPart'`,
        `'Space'`, and `'Site'` from the Arch Workbench.

        Single items that aren't groups are added to the output list
        as is.

    walls: bool, optional
        It defaults to `False`.
        If it is `True`, Wall and Structure objects (Arch Workbench)
        are treated as groups; they are scanned for Window, Door,
        and Rebar objects, and these are added to the output list.

    addgroups: bool, optional
        It defaults to `False`.
        If it is `True`, the group itself is kept as part of the output list.

    spaces: bool, optional
        It defaults to `False`.
        If it is `True`, Arch Spaces are treated as groups,
        and are added to the output list.

    noarchchild: bool, optional
        It defaults to `False`.
        If it is `True`, the objects inside Building and BuildingParts
        (Arch Workbench) aren't added to the output list.

    Returns
    -------
    list
        The list of objects from each group present in `objectslist`,
        plus any other individual object given in `objectslist`.
    """
    newlist = []
    if not isinstance(objectslist, list):
        objectslist = [objectslist]

    for obj in objectslist:
        item = obj
        if isinstance(item, tuple):
            obj = item[0].getSubObject(item[1], retType=1)
        if obj:
            children = []
            if (obj.isDerivedFrom("App::DocumentObjectGroup")
                    or (utils.get_type(obj) in ("Building", "BuildingPart",
                                                "Space", "Site")
                        and hasattr(obj, "Group"))):
                if utils.get_type(obj) == "Site":
                    if obj.Shape:
                        newlist.append(item)
                if obj.isDerivedFrom("Drawing::FeaturePage"):
                    # Skip if the group is a Drawing page
                    # Note: the Drawing workbench is obsolete
                    newlist.append(item)
                    continue
                else:
                    if addgroups or (spaces
                                     and utils.get_type(obj) == "Space"):
                        newlist.append(item)
                    if (noarchchild
                            and utils.get_type(obj) in ("Building",
                                                        "BuildingPart")):
                        continue
                    else:
                        children = obj.Group

            if not children:
                subs = obj.getSubObjects()
                for sub in subs:
                    if obj is not item:
                        children.append((item[0], item[1]+sub))
                    else:
                        sobj = obj.getSubObject(sub, retType=1)
                        if sobj:
                            children.append(sobj)
            if children:
                newlist += get_group_contents(children, walls, addgroups)
            elif not newlist or newlist[-1] is not item:
                # print("adding ", obj.Name)
                newlist.append(item)
                if walls:
                    newlist.extend(get_windows(item))

    # Clean possible duplicates
    cleanlist = []
    for obj in newlist:
        if obj not in cleanlist:
            cleanlist.append(obj)

    return cleanlist


def getGroupContents(objectslist,
                     walls=False, addgroups=False,
                     spaces=False, noarchchild=False):
    """Return a list of objects from groups. DEPRECATED."""
    utils.use_instead("get_group_contents")

    return get_group_contents(objectslist,
                              walls, addgroups,
                              spaces, noarchchild)


def get_movable_children(objectslist, recursive=True):
    """Return a list of objects with child objects that move with a host.

    Builds a list of objects with all child objects (`obj.OutList`)
    that have their `MoveWithHost` attribute set to `True`.
    This function is mostly useful for Arch Workbench objects.

    Parameters
    ----------
    objectslist: list of App::DocumentObject
        A single scripted object or list of objects.

    recursive: bool, optional
        It defaults to `True`, in which case the function
        is called recursively to also extract the children of children
        objects.
        Otherwise, only direct children of the input objects
        are added to the output list.

    Returns
    -------
    list
        List of children objects that have their `MoveWithHost` attribute
        set to `True`.
    """
    added = []
    if not isinstance(objectslist, list):
        objectslist = [objectslist]

    for obj in objectslist:
        # Skips some objects that should never move their children
        if utils.get_type(obj) not in ("Clone", "SectionPlane",
                                       "Facebinder", "BuildingPart"):
            children = obj.OutList
            if (hasattr(obj, "Proxy") and obj.Proxy
                    and hasattr(obj.Proxy, "getSiblings")
                    and utils.get_type(obj) != "Window"):
                # children.extend(obj.Proxy.getSiblings(obj))
                pass

            for child in children:
                if hasattr(child, "MoveWithHost") and child.MoveWithHost:
                    if hasattr(obj, "CloneOf"):
                        if obj.CloneOf:
                            if obj.CloneOf.Name != child.Name:
                                added.append(child)
                        else:
                            added.append(child)
                    else:
                        added.append(child)

            if recursive:
                added.extend(get_movable_children(children))

    return added


def getMovableChildren(objectslist, recursive=True):
    """Return a list of objects with child objects. DEPRECATED."""
    utils.use_instead("get_movable_children")
    return get_movable_children(objectslist, recursive)

## @}
