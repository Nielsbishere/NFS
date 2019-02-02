#pragma once
#include <QtWidgets/qwidget.h>
#include <filesystem.h>
#include "infowindow.h"

namespace nfsu {

	class NExplorer;

	class Window : public QWidget {

	public:

		Window();
		~Window();

		///UI actions

		void setupUI();
		void setupLayout();
		void setupToolbar();
		void setupExplorer(QLayout *layout);
		void setupInfoWindow(QLayout *layout);

		///Toolbar actions

			///File actions

			void load();
			void load(QString file);

			void reload();

			void write();
			void write(QString file);

			void exportPatch();
			void exportPatch(QString file);

			void importPatch();
			void importPatch(QString file);

			void findFile();
			void filterFiles();
			void orderFiles();

			///View actions
			void restore();

			///Help actions

			void documentation();

		///Right click resource actions

			void activateResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao, const QPoint &point);

			void viewResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void viewData(Buffer buf);
			void exportResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void importResource(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void info(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);

			void inspect(nfs::FileSystemObject &fso, nfs::ArchiveObject &ao);
			void inspectFolder(nfs::FileSystemObject &fso);

	private:

		QString file;
		Buffer rom;
		nfs::FileSystem fileSystem;

		nfsu::NExplorer *explorer = nullptr;
		nfsu::InfoWindow *fileInspect = nullptr;

		QLayout *layout = nullptr;

	};

}