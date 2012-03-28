/***************************************************************************
 *   Copyright (C) 2008 by Jean-Baptiste Mardelle (jb@kdenlive.org)        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#include "collapsiblegroup.h"


#include <QMenu>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QDragEnterEvent>
#include <QDropEvent>

#include <KDebug>
#include <KGlobalSettings>
#include <KLocale>
#include <KMessageBox>
#include <KStandardDirs>
#include <KFileDialog>
#include <KUrlRequester>
#include <KColorScheme>


CollapsibleGroup::CollapsibleGroup(int ix, bool firstGroup, bool lastGroup, QWidget * parent) :
        AbstractCollapsibleWidget(parent),
        m_index(ix)
{
    setupUi(this);
    setFont(KGlobalSettings::smallestReadableFont());
   
    buttonUp->setIcon(KIcon("kdenlive-up"));
    buttonUp->setToolTip(i18n("Move effect up"));
    buttonDown->setIcon(KIcon("kdenlive-down"));
    buttonDown->setToolTip(i18n("Move effect down"));

    buttonDel->setIcon(KIcon("kdenlive-deleffect"));
    buttonDel->setToolTip(i18n("Delete effect"));
    if (firstGroup) buttonUp->setVisible(false);
    if (lastGroup) buttonDown->setVisible(false);
    m_menu = new QMenu;
    m_menu->addAction(KIcon("view-refresh"), i18n("Reset effect"), this, SLOT(slotResetEffect()));
    m_menu->addAction(KIcon("document-save"), i18n("Save effect"), this, SLOT(slotSaveEffect()));
    
    title->setText(i18n("Effect Group"));
    effecticon->setPixmap(KIcon("folder").pixmap(16,16));
    m_menu->addAction(KIcon("list-remove"), i18n("Ungroup"), this, SLOT(slotUnGroup()));
    setAcceptDrops(true);
    menuButton->setIcon(KIcon("kdenlive-menu"));
    menuButton->setMenu(m_menu);
    
    enabledBox->setChecked(true);

    connect(collapseButton, SIGNAL(clicked()), this, SLOT(slotSwitch()));
    connect(enabledBox, SIGNAL(toggled(bool)), this, SLOT(slotEnable(bool)));
    connect(buttonUp, SIGNAL(clicked()), this, SLOT(slotEffectUp()));
    connect(buttonDown, SIGNAL(clicked()), this, SLOT(slotEffectDown()));
    connect(buttonDel, SIGNAL(clicked()), this, SLOT(slotDeleteEffect()));

}

CollapsibleGroup::~CollapsibleGroup()
{
    delete m_menu;
}

void CollapsibleGroup::slotUnGroup()
{
    emit unGroup(this);
}

bool CollapsibleGroup::isActive() const
{
    return decoframegroup->property("active").toBool();
}

void CollapsibleGroup::setActive(bool activate)
{
    decoframegroup->setProperty("active", activate);
    decoframegroup->setStyleSheet(decoframegroup->styleSheet());
}

void CollapsibleGroup::mouseDoubleClickEvent ( QMouseEvent * event )
{
    if (frame->underMouse() && collapseButton->isEnabled()) slotSwitch();
    QWidget::mouseDoubleClickEvent(event);
}


void CollapsibleGroup::slotEnable(bool enable)
{
    title->setEnabled(enable);
    enabledBox->blockSignals(true);
    enabledBox->setChecked(enable);
    enabledBox->blockSignals(false);
    QVBoxLayout *vbox = static_cast<QVBoxLayout *>(widgetFrame->layout());
    if (vbox == NULL) return;
    for (int i = 0; i < vbox->count(); i++) {
	CollapsibleGroup *e = static_cast<CollapsibleGroup *>(vbox->itemAt(i)->widget());
	if (e) e->enabledBox->setChecked(enable);// slotEnable(enable);
    }
}

void CollapsibleGroup::slotDeleteEffect()
{
    emit deleteGroup(groupIndex());
}

void CollapsibleGroup::slotEffectUp()
{
    emit changeGroupPosition(groupIndex(), true);
}

void CollapsibleGroup::slotEffectDown()
{
    emit changeGroupPosition(groupIndex(), false);
}

void CollapsibleGroup::slotSaveEffect()
{
    QString name = QInputDialog::getText(this, i18n("Save Effect"), i18n("Name for saved effect: "));
    if (name.isEmpty()) return;
    QString path = KStandardDirs::locateLocal("appdata", "effects/", true);
    path = path + name + ".xml";
    if (QFile::exists(path)) if (KMessageBox::questionYesNo(this, i18n("File %1 already exists.\nDo you want to overwrite it?", path)) == KMessageBox::No) return;

    /*TODO
    QDomDocument doc;
    QDomElement effect = m_effect.cloneNode().toElement();
    doc.appendChild(doc.importNode(effect, true));
    effect = doc.firstChild().toElement();
    effect.removeAttribute("kdenlive_ix");
    effect.setAttribute("id", name);
    effect.setAttribute("type", "custom");
    QDomElement effectname = effect.firstChildElement("name");
    effect.removeChild(effectname);
    effectname = doc.createElement("name");
    QDomText nametext = doc.createTextNode(name);
    effectname.appendChild(nametext);
    effect.insertBefore(effectname, QDomNode());
    QDomElement effectprops = effect.firstChildElement("properties");
    effectprops.setAttribute("id", name);
    effectprops.setAttribute("type", "custom");

    QFile file(path);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&file);
        out << doc.toString();
    }
    file.close();
    emit reloadEffects();*/
}

void CollapsibleGroup::slotResetEffect()
{
    //TODO: emit resetEffect(effectIndex());
}

void CollapsibleGroup::slotSwitch()
{
    bool enable = !widgetFrame->isVisible();
    slotShow(enable);
}

void CollapsibleGroup::slotShow(bool show)
{
    widgetFrame->setVisible(show);
    if (show) {
        collapseButton->setArrowType(Qt::DownArrow);
	m_info.isCollapsed = false;
    }
    else {
        collapseButton->setArrowType(Qt::RightArrow);
	m_info.isCollapsed = true;
    }
    //emit parameterChanged(m_original_effect, m_effect, effectIndex());   
}

void CollapsibleGroup::updateGroupIndex(int groupIndex)
{
  /*TODO:
    m_info.groupIndex = groupIndex;
    m_effect.setAttribute("kdenlive_info", m_info.toString());
    emit parameterChanged(m_original_effect, m_effect, effectIndex());*/
}

void CollapsibleGroup::addGroupEffect(CollapsibleEffect *effect)
{
    QVBoxLayout *vbox = static_cast<QVBoxLayout *>(widgetFrame->layout());
    if (vbox == NULL) {
	vbox = new QVBoxLayout();
	vbox->setContentsMargins(10, 0, 0, 0);
	vbox->setSpacing(2);
	widgetFrame->setLayout(vbox);
    }
    effect->setGroupIndex(groupIndex());
    vbox->addWidget(effect);
}

QString CollapsibleGroup::infoString() const
{
    return m_info.toString();
}

void CollapsibleGroup::removeGroup(int ix, QVBoxLayout *layout)
{
    QVBoxLayout *vbox = static_cast<QVBoxLayout *>(widgetFrame->layout());
    if (vbox == NULL) return;
    
    for (int j = vbox->count() - 1; j >= 0; j--) {
	QLayoutItem *child = vbox->takeAt(j);
	CollapsibleGroup *e = static_cast<CollapsibleGroup *>(child->widget());
	layout->insertWidget(ix, e);
	e->updateGroupIndex(-1);
	delete child;
    }
}

int CollapsibleGroup::groupIndex() const
{
    return m_index;
}



bool CollapsibleGroup::isGroup() const
{
    return true;
}

void CollapsibleGroup::updateTimecodeFormat()
{
    QVBoxLayout *vbox = static_cast<QVBoxLayout *>(widgetFrame->layout());
    if (vbox == NULL) return;
    for (int j = vbox->count() - 1; j >= 0; j--) {
	CollapsibleEffect *e = static_cast<CollapsibleEffect *>(vbox->itemAt(j)->widget());
	if (e) e->updateTimecodeFormat();
    }
}

void CollapsibleGroup::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("kdenlive/effectslist")) {
	frame->setProperty("active", true);
	frame->setStyleSheet(frame->styleSheet());
	event->acceptProposedAction();
    }
}

void CollapsibleGroup::dragLeaveEvent(QDragLeaveEvent */*event*/)
{
    frame->setProperty("active", false);
    frame->setStyleSheet(frame->styleSheet());
}

void CollapsibleGroup::dropEvent(QDropEvent *event)
{
    frame->setProperty("active", false);
    frame->setStyleSheet(frame->styleSheet());
    const QString effects = QString::fromUtf8(event->mimeData()->data("kdenlive/effectslist"));
    //event->acceptProposedAction();
    QDomDocument doc;
    doc.setContent(effects, true);
    QDomElement e = doc.documentElement();
    int ix = e.attribute("kdenlive_ix").toInt();
    if (ix == 0) {
	// effect dropped from effects list, add it
	e.setAttribute("kdenlive_ix", ix);
	event->setDropAction(Qt::CopyAction);
	event->accept();
	emit addEffect(e);
	return;
    }
    int new_index = -1;
    QVBoxLayout *vbox = static_cast<QVBoxLayout *>(widgetFrame->layout());
    if (vbox == NULL) return;
    CollapsibleEffect *effect = static_cast<CollapsibleEffect *>(vbox->itemAt(vbox->count() -1)->widget());
    new_index = effect->effectIndex();
    emit moveEffect(ix, new_index, m_index);
    event->setDropAction(Qt::MoveAction);
    event->accept();
}

