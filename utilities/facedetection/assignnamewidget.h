/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-12
 * Description : Widget for assignment and confirmation of names for faces
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ASSIGNNAMEWIDGET_H
#define ASSIGNNAMEWIDGET_H

// Qt includes

#include <QFrame>
#include <QVariant>

namespace Digikam
{

class AddTagsComboBox;
class AddTagsLineEdit;
class CheckableAlbumFilterModel;
class DatabaseFace;
class ImageInfo;
class TAlbum;
class TaggingAction;
class TagModel;
class TagPropertiesFilterModel;

class AssignNameWidget : public QFrame
{
    Q_OBJECT
    Q_ENUMS(Mode TagEntryWidgetMode LayoutMode VisualStyle)
    Q_PROPERTY(Mode mode READ mode WRITE setMode)
    Q_PROPERTY(TagEntryWidgetMode tagEntryWidgetMode READ tagEntryWidgetMode WRITE setTagEntryWidgetMode)
    Q_PROPERTY(LayoutMode layoutMode READ layoutMode WRITE setLayoutMode)
    Q_PROPERTY(VisualStyle visualStyle READ visualStyle WRITE setVisualStyle)

public:

    enum Mode
    {
        InvalidMode,
        UnconfirmedEditMode,
        ConfirmedMode,
        ConfirmedEditMode
    };

    enum TagEntryWidgetMode
    {
        InvalidTagEntryWidgetMode,
        AddTagsComboBoxMode,
        AddTagsLineEditMode
    };

    enum LayoutMode
    {
        InvalidLayout,
        FullLine,
        TwoLines,
        Compact
    };

    enum VisualStyle
    {
        InvalidVisualStyle,
        StyledFrame,
        TranslucentDarkRound,
        TranslucentThemedFrameless
    };

    /** Please take care: you must set all four modes before usage! */

    AssignNameWidget(QWidget* parent = 0);
    ~AssignNameWidget();

    /** Set the tag model to use for completion. */
    void setModel(TagModel* model, TagPropertiesFilterModel* filteredModel, CheckableAlbumFilterModel* filterModel);
    void setDefaultModel();

    void setMode(Mode mode);
    Mode mode() const;

    void setTagEntryWidgetMode(TagEntryWidgetMode mode);
    TagEntryWidgetMode tagEntryWidgetMode() const;

    void setLayoutMode(LayoutMode mode);
    LayoutMode layoutMode() const;

    void setVisualStyle(VisualStyle style);
    VisualStyle visualStyle() const;

    ImageInfo info() const;
    QVariant  faceIdentifier() const;

    /// The combo box or line edit in use, if any
    AddTagsComboBox* comboBox() const;
    AddTagsLineEdit* lineEdit() const;

public Q_SLOTS:

    /** The identifying information emitted with the signals */
    void setUserData(const ImageInfo& info, const QVariant& faceIdentifier = QVariant());

    /** Sets the suggested (UnconfirmedEditMode) or assigned (ConfirmedMode) tag to be displayed. */
    void setCurrentTag(int tagId);
    void setCurrentTag(TAlbum* album);
    void setCurrentFace(const DatabaseFace& face);

    /** Set a parent tag for suggesting a parent tag for a new tag, and a default action. */
    void setParentTag(TAlbum* album);

Q_SIGNALS:

    /**
     * A name has been assigned to the associated face.
     * This can be an existing tag, or a new tag, as described by TaggingAction.
     * For convenience, info() and faceIdentifier() are provided.
     */
    void assigned(const TaggingAction& action, const ImageInfo& info, const QVariant& faceIdentifier);

    /**
     * The face has been reject and shall be removed.
     * For convenience, info() and faceIdentifier() are provided.
     */
    void rejected(const ImageInfo& info, const QVariant& faceIdentifier);

    /**
     * In ConfirmedMode, this signal is emitted when the user clicked on the label
     */
    void labelClicked(const ImageInfo& info, const QVariant& faceIdentifier);

protected:

    void keyPressEvent(QKeyEvent* e);

protected Q_SLOTS:

    void slotConfirm();
    void slotReject();
    void slotActionActivated(const TaggingAction& action);
    void slotActionSelected(const TaggingAction& action);
    void slotLabelClicked();

private:

    class AssignNameWidgetPriv;
    AssignNameWidgetPriv* const d;
};

} // namespace Digikam

#endif // ASSIGNNAMEWIDGET_H