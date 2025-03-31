#include "Logger.h"

Logger::Logger() {
    const auto logDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DAW_Prototype")
        .getChildFile("logs");
        
    logDir.createDirectory();
    
    const auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
    const auto logFile = logDir.getChildFile("daw_" + timestamp + ".log");
    
    setLogFile(logFile);
    
    LOG_INFO("Logger initialized");
    LOG_INFO("Log file: %s", logFile.getFullPathName().toRawUTF8());
}

Logger::~Logger() {
    closeLogFile();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLogFile(const juce::File& file) {
    const juce::ScopedLock lock(logLock);
    
    closeLogFile();
    currentLogFile = file;
    openLogFile();
    
    // Write header to new log file
    const auto header = juce::String("DAW Prototype Log - Started at ")
        + juce::Time::getCurrentTime().toString(true, true, true, true)
        + juce::newLine
        + "----------------------------------------"
        + juce::newLine;
        
    writeToLog(header);
}

void Logger::closeLogFile() {
    const juce::ScopedLock lock(logLock);
    
    if (logStream != nullptr) {
        logStream->flush();
        logStream = nullptr;
    }
}

void Logger::logMessageInternal(Level level, const juce::String& message) {
    if (level < minimumLevel) {
        return;
    }

    const juce::ScopedLock lock(logLock);
    
    const auto formattedMessage = getTimestamp()
        + " [" + getLevelString(level) + "] "
        + message
        + juce::newLine;
    
    // Write to console if enabled
    if (consoleOutputEnabled) {
        if (level == Level::Error) {
            std::cerr << formattedMessage << std::flush;
        } else {
            std::cout << formattedMessage << std::flush;
        }
    }
    
    // Write to log file
    writeToLog(formattedMessage);
    
    // Check if log rotation is needed
    checkLogRotation();
}

juce::String Logger::getLevelString(Level level) const {
    return levelStrings[static_cast<int>(level)];
}

juce::String Logger::getTimestamp() const {
    return juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S.%03d");
}

void Logger::openLogFile() {
    if (currentLogFile != juce::File()) {
        logStream = std::make_unique<juce::FileOutputStream>(currentLogFile);
        
        if (logStream->failedToOpen()) {
            std::cerr << "Failed to open log file: "
                     << currentLogFile.getFullPathName() << std::endl;
            logStream = nullptr;
        }
    }
}

void Logger::writeToLog(const juce::String& text) {
    if (logStream != nullptr && !logStream->failedToOpen()) {
        logStream->writeText(text, false, false);
        logStream->flush();
    }
}

void Logger::checkLogRotation() {
    if (logStream != nullptr) {
        const auto size = currentLogFile.getSize();
        
        if (size > maxLogSize) {
            forceRotateLog();
        }
    }
}

void Logger::rotateLogIfNeeded() {
    const juce::ScopedLock lock(logLock);
    
    if (currentLogFile.exists()) {
        const auto size = currentLogFile.getSize();
        const auto age = juce::Time::getCurrentTime().toMilliseconds() -
                        currentLogFile.getCreationTime().toMilliseconds();
        const auto ageInDays = age / (1000 * 60 * 60 * 24);
        
        if (size > maxLogSize || ageInDays > maxLogAge) {
            forceRotateLog();
        }
    }
}

void Logger::forceRotateLog() {
    const juce::ScopedLock lock(logLock);
    
    // Close current log file
    closeLogFile();
    
    // Create new log file with timestamp
    const auto logDir = currentLogFile.getParentDirectory();
    const auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
    const auto newLogFile = logDir.getChildFile("daw_" + timestamp + ".log");
    
    // Move current log to timestamped file
    currentLogFile.moveFileTo(newLogFile);
    
    // Open new log file
    openLogFile();
    
    LOG_INFO("Log rotated to: %s", newLogFile.getFullPathName().toRawUTF8());
    
    // Clean old logs
    cleanOldLogs();
}

void Logger::cleanOldLogs() {
    const auto logDir = currentLogFile.getParentDirectory();
    const auto logs = logDir.findChildFiles(juce::File::findFiles, false, "*.log");
    
    const auto now = juce::Time::getCurrentTime();
    const auto maxAgeMs = static_cast<int64_t>(maxLogAge) * 24 * 60 * 60 * 1000;
    
    for (const auto& log : logs) {
        // Skip current log file
        if (log == currentLogFile) {
            continue;
        }
        
        const auto age = now.toMilliseconds() - log.getCreationTime().toMilliseconds();
        
        if (age > maxAgeMs) {
            log.deleteFile();
            LOG_INFO("Deleted old log file: %s", log.getFullPathName().toRawUTF8());
        }
    }
}