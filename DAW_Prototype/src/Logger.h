#pragma once
#include <JuceHeader.h>

// Logging macros for convenience
#define LOG_INFO(...)    Logger::getInstance().logMessage(Logger::Level::Info, __VA_ARGS__)
#define LOG_WARNING(...) Logger::getInstance().logMessage(Logger::Level::Warning, __VA_ARGS__)
#define LOG_ERROR(...)   Logger::getInstance().logMessage(Logger::Level::Error, __VA_ARGS__)
#define LOG_DEBUG(...)   Logger::getInstance().logMessage(Logger::Level::Debug, __VA_ARGS__)

class Logger : public juce::DeletedAtShutdown {
public:
    // Log levels
    enum class Level {
        Debug,
        Info,
        Warning,
        Error
    };

    // Constructor/Destructor
    Logger();
    ~Logger() override;

    // Singleton access
    static Logger& getInstance();

    // Logging methods
    template<typename... Args>
    void logMessage(Level level, const char* format, Args... args) {
        const juce::String message = juce::String::formatted(format, args...);
        logMessageInternal(level, message);
    }

    void logMessage(Level level, const juce::String& message) {
        logMessageInternal(level, message);
    }

    // File handling
    void setLogFile(const juce::File& file);
    void closeLogFile();
    juce::File getLogFile() const { return currentLogFile; }

    // Log level control
    void setMinimumLevel(Level level) { minimumLevel = level; }
    Level getMinimumLevel() const { return minimumLevel; }

    // Log retention
    void setMaxLogSize(int64_t bytes) { maxLogSize = bytes; }
    void setMaxLogAge(int days) { maxLogAge = days; }
    void cleanOldLogs();

    // Console output control
    void enableConsoleOutput(bool enable) { consoleOutputEnabled = enable; }
    bool isConsoleOutputEnabled() const { return consoleOutputEnabled; }

    // Log rotation
    void rotateLogIfNeeded();
    void forceRotateLog();

private:
    void logMessageInternal(Level level, const juce::String& message);
    juce::String getLevelString(Level level) const;
    juce::String getTimestamp() const;
    void openLogFile();
    void writeToLog(const juce::String& text);
    void checkLogRotation();
    
    juce::CriticalSection logLock;
    std::unique_ptr<juce::FileOutputStream> logStream;
    juce::File currentLogFile;
    Level minimumLevel{Level::Info};
    int64_t maxLogSize{10 * 1024 * 1024}; // 10MB
    int maxLogAge{30}; // 30 days
    bool consoleOutputEnabled{true};
    
    static inline const char* const levelStrings[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR"
    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Logger)
};