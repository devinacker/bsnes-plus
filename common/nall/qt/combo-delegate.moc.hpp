#ifndef NALL_QT_COMBODELEGATE_HPP
#define NALL_QT_COMBODELEGATE_HPP

namespace nall {

class ComboDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  ComboDelegate(const QStringList& strings, QObject *parent = 0);
  
  QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private slots:
  void commit();

private:
  QStringList strings;
};

inline ComboDelegate::ComboDelegate(const QStringList& strings, QObject *parent)
  : QStyledItemDelegate(parent)
  , strings(strings) {
}

inline QWidget* ComboDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const {
  QComboBox *combo = new QComboBox(parent);
  combo->addItems(strings);
  combo->setCurrentIndex(index.model()->data(index, Qt::EditRole).toInt());
  connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commit()));
  return combo;
}

inline void ComboDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  QStyledItemDelegate::updateEditorGeometry(editor, option, index);

  QComboBox *combo = qobject_cast<QComboBox*>(editor);
  combo->showPopup();
}

inline void ComboDelegate::commit() {
  QWidget *widget = qobject_cast<QWidget*>(sender());
  emit commitData(widget);
}

inline void ComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
}

inline void ComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  QComboBox *combo = qobject_cast<QComboBox*>(editor);
  model->setData(index, combo->currentIndex());
}

}

#endif
