#ifndef TASKSCRIPT_MAIN_WINDOW_H
#define TASKSCRIPT_MAIN_WINDOW_H

#include <QMainWindow>
#include <QString>

#include "core/BoardModel.h"
#include "core/ErrorManager.h"
#include "core/ParseTree.h"
#include "core/Token.h"

class QPlainTextEdit;
class QTableWidget;
class QLabel;
class QPushButton;

namespace taskscript {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onLoadFile();
    void onSaveFile();
    void onAnalyze();
    void onGenerateReports();
    void onGenerateDot();
    void onOpenOutputFolder();

private:
    void buildUi();
    void analyzeCurrentSource();
    void fillTokenTable(const std::vector<Token>& tokens);
    void fillErrorTable(const std::vector<AnalysisError>& errors);
    QString outputDirectory() const;
    void ensureOutputDirectory();
    void setStatus(const QString& message, bool error = false);

    QPlainTextEdit* sourceEdit_ = nullptr;
    QTableWidget* tokenTable_ = nullptr;
    QTableWidget* errorTable_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* fileLabel_ = nullptr;

    QPushButton* btnReports_ = nullptr;
    QPushButton* btnDot_ = nullptr;

    QString currentFilePath_;
    bool analyzed_ = false;

    std::vector<Token> lastTokens_;
    ParseNodePtr lastTree_;
    Tablero lastBoard_;
    ErrorManager lastErrors_;
};

}  // namespace taskscript

#endif  // TASKSCRIPT_MAIN_WINDOW_H
