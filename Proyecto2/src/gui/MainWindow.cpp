#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QString>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <fstream>
#include <sstream>

#include "core/LexicalAnalyzer.h"
#include "core/ReportGenerator.h"
#include "core/SyntaxAnalyzer.h"

namespace taskscript {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    buildUi();
    setStatus(tr("Listo. Cargue un archivo .task o escriba en el editor."));
}

void MainWindow::buildUi() {
    setWindowTitle(tr("TaskScript - Analizador Lexico y Sintactico"));
    resize(1180, 780);

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(6);

    // Barra superior con archivo cargado.
    fileLabel_ = new QLabel(tr("Archivo: (sin cargar)"), central);
    fileLabel_->setStyleSheet("color:#7F8C8D;");
    root->addWidget(fileLabel_);

    auto* splitter = new QSplitter(Qt::Vertical, central);

    // Editor.
    sourceEdit_ = new QPlainTextEdit(splitter);
    sourceEdit_->setPlaceholderText(tr("Escriba o pegue aqui el codigo TaskScript..."));
    QFont mono("Menlo");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(12);
    sourceEdit_->setFont(mono);

    // Tabs de tokens / errores.
    auto* tabs = new QTabWidget(splitter);

    tokenTable_ = new QTableWidget(0, 5, tabs);
    tokenTable_->setHorizontalHeaderLabels(
        {tr("No."), tr("Lexema"), tr("Tipo"), tr("Linea"), tr("Columna")});
    tokenTable_->horizontalHeader()->setStretchLastSection(true);
    tokenTable_->verticalHeader()->setVisible(false);
    tokenTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tokenTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tokenTable_->setAlternatingRowColors(true);
    tabs->addTab(tokenTable_, tr("Tokens"));

    errorTable_ = new QTableWidget(0, 7, tabs);
    errorTable_->setHorizontalHeaderLabels(
        {tr("No."), tr("Lexema"), tr("Tipo"), tr("Descripcion"),
         tr("Linea"), tr("Columna"), tr("Gravedad")});
    errorTable_->horizontalHeader()->setStretchLastSection(true);
    errorTable_->verticalHeader()->setVisible(false);
    errorTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    errorTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    errorTable_->setAlternatingRowColors(true);
    tabs->addTab(errorTable_, tr("Errores"));

    splitter->addWidget(sourceEdit_);
    splitter->addWidget(tabs);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    root->addWidget(splitter, 1);

    setCentralWidget(central);

    // Barra de estado.
    statusLabel_ = new QLabel(this);
    statusBar()->addWidget(statusLabel_, 1);

    // Toolbar con botones grandes.
    auto* toolbar = addToolBar(tr("Acciones"));
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(18, 18));

    auto* actLoad = toolbar->addAction(tr("Cargar archivo"));
    auto* actSave = toolbar->addAction(tr("Guardar como"));
    toolbar->addSeparator();
    auto* actAnalyze = toolbar->addAction(tr("Analizar"));
    toolbar->addSeparator();
    auto* actReports = toolbar->addAction(tr("Generar reportes HTML"));
    auto* actDot = toolbar->addAction(tr("Generar arbol DOT"));
    toolbar->addSeparator();
    auto* actOpenOut = toolbar->addAction(tr("Abrir carpeta de salida"));

    // Mantener referencias a los botones que dependen del analisis exitoso.
    btnReports_ = nullptr;
    btnDot_ = nullptr;
    actReports->setEnabled(false);
    actDot->setEnabled(false);

    connect(actLoad, &QAction::triggered, this, &MainWindow::onLoadFile);
    connect(actSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(actAnalyze, &QAction::triggered, this, [this, actReports, actDot]() {
        onAnalyze();
        const bool ok = analyzed_ && !lastErrors_.hasErrors();
        actReports->setEnabled(ok);
        actDot->setEnabled(analyzed_);
    });
    connect(actReports, &QAction::triggered, this, &MainWindow::onGenerateReports);
    connect(actDot, &QAction::triggered, this, &MainWindow::onGenerateDot);
    connect(actOpenOut, &QAction::triggered, this, &MainWindow::onOpenOutputFolder);

    // Menu File para accesos comunes.
    auto* menuArchivo = menuBar()->addMenu(tr("Archivo"));
    menuArchivo->addAction(actLoad);
    menuArchivo->addAction(actSave);
    menuArchivo->addSeparator();
    auto* actQuit = menuArchivo->addAction(tr("Salir"));
    connect(actQuit, &QAction::triggered, this, &QMainWindow::close);

    auto* menuAnalisis = menuBar()->addMenu(tr("Analisis"));
    menuAnalisis->addAction(actAnalyze);
    menuAnalisis->addAction(actReports);
    menuAnalisis->addAction(actDot);

    auto* menuAyuda = menuBar()->addMenu(tr("Ayuda"));
    auto* actAbout = menuAyuda->addAction(tr("Acerca de"));
    connect(actAbout, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, tr("Acerca de TaskScript"),
            tr("TaskScript - Proyecto 2\n"
               "Lenguajes Formales y de Programacion\n"
               "Universidad de San Carlos de Guatemala\n\n"
               "Analizador lexico (AFD manual) y parser descendente recursivo.\n"
               "Genera reportes HTML del tablero Kanban y arbol de derivacion en Graphviz."));
    });
}

void MainWindow::onLoadFile() {
    QString path = QFileDialog::getOpenFileName(
        this, tr("Abrir archivo .task"), QString(),
        tr("Archivos TaskScript (*.task);;Todos los archivos (*)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("No se pudo abrir el archivo:\n%1").arg(path));
        return;
    }
    QTextStream in(&f);
    sourceEdit_->setPlainText(in.readAll());
    currentFilePath_ = path;
    fileLabel_->setText(tr("Archivo: %1").arg(QFileInfo(path).fileName()));
    setStatus(tr("Archivo cargado: %1").arg(path));
    analyzed_ = false;
}

void MainWindow::onSaveFile() {
    QString path = QFileDialog::getSaveFileName(
        this, tr("Guardar como"), currentFilePath_,
        tr("Archivos TaskScript (*.task);;Todos los archivos (*)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("No se pudo escribir el archivo:\n%1").arg(path));
        return;
    }
    QTextStream out(&f);
    out << sourceEdit_->toPlainText();
    currentFilePath_ = path;
    fileLabel_->setText(tr("Archivo: %1").arg(QFileInfo(path).fileName()));
    setStatus(tr("Archivo guardado: %1").arg(path));
}

void MainWindow::onAnalyze() {
    analyzeCurrentSource();
}

void MainWindow::analyzeCurrentSource() {
    QString src = sourceEdit_->toPlainText();
    lastErrors_.clear();
    LexicalAnalyzer lexer(src.toStdString(), lastErrors_);
    lastTokens_ = lexer.tokenizeAll();

    SyntaxAnalyzer parser(lastTokens_, lastErrors_);
    lastTree_ = parser.parse();
    lastBoard_ = parser.board();

    fillTokenTable(lastTokens_);
    fillErrorTable(lastErrors_.errors());

    analyzed_ = true;
    if (lastErrors_.hasErrors()) {
        setStatus(tr("Analisis finalizado con %1 error(es). Vea la pestana Errores.")
                      .arg(lastErrors_.errors().size()),
                  true);
    } else {
        setStatus(tr("Analisis exitoso: %1 tokens, sin errores.")
                      .arg(lastTokens_.size()));
    }
}

void MainWindow::fillTokenTable(const std::vector<Token>& tokens) {
    tokenTable_->setRowCount(0);
    int row = 0;
    for (const auto& t : tokens) {
        if (t.type() == TokenType::END_OF_FILE) continue;
        tokenTable_->insertRow(row);
        tokenTable_->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        tokenTable_->setItem(row, 1,
                              new QTableWidgetItem(QString::fromStdString(t.lexeme())));
        tokenTable_->setItem(row, 2,
                              new QTableWidgetItem(QString::fromStdString(tokenTypeToString(t.type()))));
        tokenTable_->setItem(row, 3, new QTableWidgetItem(QString::number(t.line())));
        tokenTable_->setItem(row, 4, new QTableWidgetItem(QString::number(t.column())));
        ++row;
    }
    tokenTable_->resizeColumnsToContents();
}

void MainWindow::fillErrorTable(const std::vector<AnalysisError>& errors) {
    errorTable_->setRowCount(0);
    int row = 0;
    for (const auto& e : errors) {
        errorTable_->insertRow(row);
        errorTable_->setItem(row, 0, new QTableWidgetItem(QString::number(e.number)));
        errorTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(e.lexeme)));
        errorTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(errorTypeToString(e.type))));
        errorTable_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(e.description)));
        errorTable_->setItem(row, 4, new QTableWidgetItem(QString::number(e.line)));
        errorTable_->setItem(row, 5, new QTableWidgetItem(QString::number(e.column)));
        errorTable_->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(errorSeverityToString(e.severity))));
        ++row;
    }
    errorTable_->resizeColumnsToContents();
}

QString MainWindow::outputDirectory() const {
    QString base = QCoreApplication::applicationDirPath();
    return QDir(base).filePath("salida_taskscript");
}

void MainWindow::ensureOutputDirectory() {
    QDir().mkpath(outputDirectory());
}

void MainWindow::onGenerateReports() {
    if (!analyzed_) {
        QMessageBox::information(this, tr("Atencion"), tr("Primero ejecute el analisis."));
        return;
    }
    if (lastErrors_.hasErrors()) {
        QMessageBox::warning(this, tr("Atencion"),
            tr("Existen errores en el analisis. Corrija los errores antes de generar los reportes."));
        return;
    }
    ensureOutputDirectory();
    const QString outDir = outputDirectory();
    const QString kanbanPath = QDir(outDir).filePath("Reporte_Kanban.html");
    const QString cargaPath = QDir(outDir).filePath("Reporte_CargaResponsable.html");

    bool ok1 = ReportGenerator::writeToFile(
        kanbanPath.toStdString(), ReportGenerator::generateKanbanHtml(lastBoard_));
    bool ok2 = ReportGenerator::writeToFile(
        cargaPath.toStdString(), ReportGenerator::generateCargaResponsableHtml(lastBoard_));

    if (ok1 && ok2) {
        setStatus(tr("Reportes HTML generados en: %1").arg(outDir));
        QDesktopServices::openUrl(QUrl::fromLocalFile(kanbanPath));
        QDesktopServices::openUrl(QUrl::fromLocalFile(cargaPath));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("No fue posible escribir uno o ambos reportes."));
    }
}

void MainWindow::onGenerateDot() {
    if (!analyzed_) {
        QMessageBox::information(this, tr("Atencion"), tr("Primero ejecute el analisis."));
        return;
    }
    ensureOutputDirectory();
    const QString outDir = outputDirectory();
    const QString dotPath = QDir(outDir).filePath("arbol.dot");
    bool ok = ReportGenerator::writeToFile(
        dotPath.toStdString(), ReportGenerator::generateDot(lastTree_));
    if (ok) {
        setStatus(tr("Arbol Graphviz generado en: %1").arg(dotPath));
        QDesktopServices::openUrl(QUrl::fromLocalFile(outDir));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("No fue posible escribir arbol.dot."));
    }
}

void MainWindow::onOpenOutputFolder() {
    ensureOutputDirectory();
    QDesktopServices::openUrl(QUrl::fromLocalFile(outputDirectory()));
}

void MainWindow::setStatus(const QString& message, bool error) {
    statusLabel_->setText(message);
    statusLabel_->setStyleSheet(error ? "color:#C0392B;font-weight:600;" : "color:#2C3E50;");
}

}  // namespace taskscript
