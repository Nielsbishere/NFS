#include "window.h"
#include "nexplorer.h"
#include "infowindow.h"
#include <patcher.h>
#include <QtGui/qdesktopservices.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qurl.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qlayout.h>
#include "model.h"
#include "qhelper.h"
using namespace nfsu;
using namespace nfs;

Window::Window() {

	setWindowTitle("File System Utilities");
	setMinimumSize(QSize(1200, 700));

	setStyleSheet(

		"QWidget {"
			"background: #303030;"
			"background-color: #303030;"
			"color: Cyan;"
			"border: 0px;"
			"selection-background-color: #404040;"
			"alternate-background-color: #303030;"
			"font: 14px;"
		"}"

		"QMenu {"
			"border: 1px solid #202020;"
			"background: #404040;"
		"}"

		"QTreeView::item:hover, QAbstractItemView::item:hover, QMenuBar::item:hover, QMenu::item:hover, QTabBar::tab:hover {"
			"color: DeepSkyBlue;"
		"}"

		"QTreeView::item:selected, QAbstractItemView::item:selected, QMenuBar::item:selected, QMenu::item:selected, QTabBar::tab:selected {"
			"color: White;"
			"background: #505050;"
		"}"

		"QTabWidget::pane {"
			"border: 2px solid #101010;"
		"}"

		"QTabBar::tab, QTabBar::tab:selected {"
			"background: #202020;"
			"margin-right: 5px;"
			"padding-top: 2px;"
			"padding-bottom: 2px;"
			"padding-left: 2px;"
		"}"

		"QTabBar::tab:selected {"
			"background: #505050;"
			"color: White;"
		"}"

		"QHeaderView::section {"
			"background-color: #303030;"
			"border: 0px #101010;"
		"}"

		"QTreeView, QTableView, QMenuBar {"
			"border: 1px solid #101010;"
		"}"

	);

	//TODO: Buttons

	setupUI();
}

Window::~Window() {
	rom.dealloc();
}


///UI Actions

void Window::setupUI() {
	setupLayout();
	setupToolbar();
	setupInfoWindow(leftLayout);
	setupExplorer(leftLayout);
	setupTabs(rightLayout);
}

void Window::setupLayout() {
	setLayout(layout = new QHBoxLayout);
	splitter = new QSplitter;
	layout->addWidget(splitter);
	splitter->addWidget(left = new QWidget);
	splitter->addWidget(right = new QWidget);
	left->setLayout(leftLayout = new QVBoxLayout);
	left->setMaximumWidth(450);
	right->setLayout(rightLayout = new QVBoxLayout);
}

void Window::setupToolbar() {

	QMenuBar *qtb;
	leftLayout->addWidget(qtb = new QMenuBar());

	QMenu *file = qtb->addMenu("File");
	QMenu *view = qtb->addMenu("View");
	QMenu *options = qtb->addMenu("Options");
	QMenu *help = qtb->addMenu("Help");

	///File
	QAction *load = file->addAction("Load");
	QAction *save = file->addAction("Save");
	QAction *exp = file->addAction("Export");
	QAction *imp = file->addAction("Import");
	QAction *reload = file->addAction("Reload");
	file->addSeparator();
	QAction *find = file->addAction("Find");
	QAction *filter = file->addAction("Filter");
	QAction *order = file->addAction("Order");
	QAction *mapper = file->addAction("Mapping");	//TODO: Map file spaces (find available space, etc)

	connect(load, &QAction::triggered, this, [&]() { this->load(); });
	connect(exp, &QAction::triggered, this, [&]() { this->exportPatch(); });
	connect(imp, &QAction::triggered, this, [&]() { this->importPatch(); });
	connect(reload, &QAction::triggered, this, [&]() { this->reload(); });
	connect(save, &QAction::triggered, this, [&]() { this->write(); });
	connect(find, &QAction::triggered, this, [&]() { this->findFile(); });
	connect(filter, &QAction::triggered, this, [&]() { this->filterFiles(); });
	connect(order, &QAction::triggered, this, [&]() { this->orderFiles(); });

	///View
	QAction *restore = view->addAction("Reset");
	QAction *customize = view->addAction("Customize");

	connect(restore, &QAction::triggered, this, [&]() { this->restore(); });

	///Help
	QAction *documentation = help->addAction("Documentation");
	QAction *about = help->addAction("About");

	connect(documentation, &QAction::triggered, this, [&]() {this->documentation(); });
	connect(about, &QAction::triggered, this, [&]() {this->about(); });

}

void Window::setupExplorer(QLayout *layout) {

	explorer = new NExplorer(fileSystem);

	NExplorerView *view = new NExplorerView(explorer);
	layout->addWidget(view);

	view->addResourceCallback(true, u32_MAX, [this, view](FileSystem &fs, FileSystemObject &fso, ArchiveObject &ao, const QPoint &point) {
		this->activateResource(fso, ao, view->mapToGlobal(point));
	});

	view->addExplorerCallback(false, [this](FileSystem &fs, FileSystemObject &fso, const QPoint &point) {
		if (fso.isFile())
			this->inspect(fso, fs.getResource(fso));
		else
			this->inspectFolder(fso);
	});
}

void Window::setupInfoWindow(QLayout *layout) {
	fileInspect = new InfoWindow(this);
	layout->addWidget(fileInspect);
}

void Window::setupTabs(QLayout *layout) {
	QTabWidget *tabs = new QTabWidget;
	tabs->addTab(new QWidget, QIcon("resources/folder.png"), "Game editor");			//TODO: Edit icon, names, etc.
	tabs->addTab(new QWidget, QIcon("resources/palette.png"), "Palette editor");		//TODO: Edit palette
	tabs->addTab(new QWidget, QIcon("resources/tilemap.png"), "Tile editor");			//TODO: Edit tiles
	tabs->addTab(new QWidget, QIcon("resources/map.png"), "Tilemap editor");			//TODO: Edit tilemap
	tabs->addTab(new QWidget, QIcon("resources/model.png"), "Model editor");			//TODO: Edit model
	tabs->addTab(new QWidget, QIcon("resources/binary.png"), "File editor");			//TODO: Edit binary or text
	tabs->setCurrentIndex(1);
	rightLayout->addWidget(tabs);
}

void Window::activateResource(FileSystemObject &fso, ArchiveObject &ao, const QPoint &point) {

	QMenu contextMenu(tr(ao.name.c_str()), this);

	QAction *view = contextMenu.addAction("View");
	QAction *expr = contextMenu.addAction("Export resource");
	QAction *impr = contextMenu.addAction("Import resource");
	QAction *info = contextMenu.addAction("Documentation");

	connect(view, &QAction::triggered, this, [&]() { this->viewResource(fso, ao); });
	connect(expr, &QAction::triggered, this, [&]() { this->exportResource(fso, ao); });
	connect(impr, &QAction::triggered, this, [&]() { this->importResource(fso, ao); });
	connect(info, &QAction::triggered, this, [&]() { this->info(fso, ao); });

	contextMenu.exec(point);
}

///File actions

void Window::load() {

	if (rom.ptr != nullptr) {

		QMessageBox::StandardButton reply = QMessageBox::question(this, "Load ROM", "Loading a ROM will clear all resources and not save any progress. Do you want to continue?");

		if (reply == QMessageBox::No)
			return;
	}

	QString file = QFileDialog::getOpenFileName(this, tr("Open ROM"), "", tr("NDS file (*.nds)"));
	
	if (file == "" || !file.endsWith(".nds", Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid .nds file through File->Load");
		messageBox.setFixedSize(500, 200);
		return;
	}

	load(file);
}

void Window::load(QString _file) {
	file = _file;
	reload();
}

void Window::reload() {

	fileSystem.clear();
	rom.dealloc();

	rom = Buffer::read(file.toStdString());

	if (rom.ptr != nullptr) {

		NDS *nds = (NDS*) rom.ptr;
		setWindowTitle(QString("File System Utilities: ") + nds->title);
		fileSystem = nds;

		NDSBanner *banner = NDSBanner::get(nds);
		auto strings = banner->getTitles();
		//TODO: Display, icon
		//TODO: Allow editing 
		int dbg = 0;

	}

	restore();

}

void Window::write() {

	QString file = QFileDialog::getSaveFileName(this, tr("Save ROM"), "", tr("NDS file (*.nds)"));

	if (file == "" || !file.endsWith(".nds", Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid .nds file to write to");
		messageBox.setFixedSize(500, 200);
		return;
	}

	write(file);
}

void Window::write(QString file) {
	if (rom.ptr != nullptr)
		rom.write(file.toStdString());
}

void Window::exportPatch() {
	QString file = QFileDialog::getSaveFileName(this, tr("Export Patch"), "", tr("NFS Patch file (*.NFSP)"));
	exportPatch(file);
}

void Window::exportPatch(QString file) {

	if (rom.ptr == nullptr) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a ROM before exporting a patch");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer original = Buffer::read(this->file.toStdString());

	if (original.ptr == nullptr) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Couldn't load the original ROM. Please reload the ROM");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer patch = Patcher::writePatch(original, rom);
	original.dealloc();
	
	if (patch.ptr == nullptr) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Couldn't complete patch; files were probably identical");
		messageBox.setFixedSize(500, 200);
		return;
	}

	patch.write(file.toStdString());
	patch.dealloc();
}

void Window::importPatch() {

	QMessageBox::StandardButton reply = QMessageBox::question(this, "Import Patch", "Importing a patch might damage the ROM, or might not work if applied on the wrong ROM. Do you want to continue?");

	if (reply == QMessageBox::No)
		return;

	QString file = QFileDialog::getOpenFileName(this, tr("Apply Patch"), "", tr("NFS Patch file (*.NFSP)"));

	if (file == "" || !file.endsWith(".NFSP", Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid .NFSP file through File->Apply patch");
		messageBox.setFixedSize(500, 200);
		return;
	}

	importPatch(file);
}

void Window::importPatch(QString file) {

	Buffer buf = Buffer::read(file.toStdString());

	if (buf.ptr == nullptr || rom.ptr == nullptr) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "To apply a patch, please select a valid file and ROM");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer patched = Patcher::patch(rom, buf);
	buf.dealloc();
	rom.dealloc();
	rom = patched;

	reload();
}

void Window::findFile() {
	//TODO: Example; find files with extension, name, directory, in folder, that are supported, etc.
}

void Window::filterFiles() {
	//TODO: Example; filter on extension, supported
}

void Window::orderFiles() {
	//TODO: Order on size, alphabetical, offset; ascending, descending
}

///View

void Window::restore() {

	QHelper::clearLayout(layout);
	setupUI();

	NDS *nds = (NDS*)rom.ptr;

	if (nds != nullptr) {
		fileInspect->setString("Title", nds->title);
		fileInspect->setString("Size", QString::number(nds->romSize));
		fileInspect->setString("File", file);
		fileInspect->setString("Folders", QString::number(fileSystem.getFolders()));
		fileInspect->setString("Files", QString::number(fileSystem.getFiles()));
	} else {
		fileInspect->setString("Title", "");
		fileInspect->setString("Size", "");
		fileInspect->setString("File", "");
		fileInspect->setString("Folders", "");
		fileInspect->setString("Files", "");
	}

	fileInspect->setString("Id", "");
	fileInspect->setString("Type", "");
	fileInspect->setString("Offset", "");
	fileInspect->setString("Length", "");

}

void Window::customize() {
	//TODO: Customize style sheet
}

///Help

void Window::documentation() {
	QDesktopServices::openUrl(QUrl("https://github.com/Nielsbishere/NFS/tree/NFS_Reloaded"));
}

void Window::about() {
	QDesktopServices::openUrl(QUrl("https://github.com/Nielsbishere/NFS/tree/NFS_Reloaded/nfsu"));
}

///Right click resource actions

void Window::viewResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {
	//TODO: Select all editors that can display the resource
	//TODO: Make user pick if > 0
}

void Window::viewData(Buffer buf) {
	//TODO: Select all editors that can display binary
	//TODO: Make user pick if > 0
}

void Window::exportResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	QString name = QString(ao.name.c_str()).split("/").last();
	QString extension = name.split(".").last();

	QString file = QFileDialog::getSaveFileName(this, tr("Save Resource"), "", tr((extension + " file (*." + extension + ")").toStdString().c_str()));

	if (file == "" || !file.endsWith("." + extension, Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid ." + extension + " file to write to");
		messageBox.setFixedSize(500, 200);
		return;
	}

	if (!fso.buf.write(file.toStdString())) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Couldn't write resource");
		messageBox.setFixedSize(500, 200);
		return;
	}
}

void Window::importResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	QString name = QString(ao.name.c_str()).split("/").last();
	QString extension = name.split(".").last();

	QString file = QFileDialog::getOpenFileName(this, tr("Load Resource"), "", tr((extension + " file (*." + extension + ")").toStdString().c_str()));

	if (file == "" || !file.endsWith("." + extension, Qt::CaseInsensitive)) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Please select a valid ." + extension + " file to read from");
		messageBox.setFixedSize(500, 200);
		return;
	}

	Buffer buf = Buffer::read(file.toStdString());

	if (buf.size != fso.buf.size) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Resources can't change size");
		messageBox.setFixedSize(500, 200);
		buf.dealloc();
		return;
	}

	memcpy(fso.buf.ptr, buf.ptr, buf.size);
	buf.dealloc();
}

void Window::info(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {
	if (ao.info.magicNumber != ResourceHelper::getMagicNumber<NBUO>())
		QDesktopServices::openUrl(QUrl("https://github.com/Nielsbishere/NFS/tree/NFS_Reloaded/docs/resource" + QString::number(ao.info.type) + ".md"));
	else
		QDesktopServices::openUrl(QUrl("https://github.com/Nielsbishere/NFS/tree/NFS_Reloaded/docs"));
}

void Window::inspect(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao) {

	QString fileName = QString::fromStdString(fso.name);

	fileInspect->setString("Folders", QString::number(fso.folders));
	fileInspect->setString("Files", QString::number(fso.files));

	fileInspect->setString("File", fileName);
	fileInspect->setString("Id", QString::number(fso.index));

	int occur = fileName.lastIndexOf(".");
	fileInspect->setString("Type", fileName.mid(occur + 1, fileName.size() - occur - 1));

	fileInspect->setString("Offset", QString("0x") + QString::number((u32)(fso.buf.ptr - rom.ptr), 16));
	fileInspect->setString("Length", QString::number(fso.buf.size));

}

void Window::inspectFolder(nfs::FileSystemObject &fso) {

	QString fileName = QString::fromStdString(fso.name);

	fileInspect->setString("Folders", QString::number(fso.folders));
	fileInspect->setString("Files", QString::number(fso.files));

	fileInspect->setString("File", fileName);
	fileInspect->setString("Id", QString::number(fso.index));

	fileInspect->setString("Type", "Folder");

	fileInspect->setString("Offset", "");
	fileInspect->setString("Length", "");

}