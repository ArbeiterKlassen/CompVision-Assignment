#include "pch.h"
#include "framework.h"
#include "ImageDocument.h"

ImageDocument::ImageDocument()
    : m_analyze(10, 10, CV_8UC3, cv::Scalar(39, 34, 30)),
      m_reference(10, 10, CV_8UC3, cv::Scalar(39, 34, 30))
{
}

bool ImageDocument::hasImage() const
{
    return m_loaded && !m_current.empty();
}

const cv::Mat& ImageDocument::origin() const
{
    return m_origin;
}

const cv::Mat& ImageDocument::current() const
{
    return m_current;
}

const cv::Mat& ImageDocument::analyze() const
{
    return m_analyze;
}

const cv::Mat& ImageDocument::reference() const
{
    return m_reference;
}

const CString& ImageDocument::sourcePath() const
{
    return m_sourcePath;
}

void ImageDocument::setSourcePath(const CString& path)
{
    m_sourcePath = path;
}

const CString& ImageDocument::savePath() const
{
    return m_savePath;
}

void ImageDocument::setSavePath(const CString& path)
{
    m_savePath = path;
}

const CString& ImageDocument::referencePath() const
{
    return m_refPath;
}

void ImageDocument::setReferencePath(const CString& path)
{
    m_refPath = path;
}

bool ImageDocument::open(const CString& filePath)
{
    std::string path = CT2A(filePath.GetString());
    cv::Mat img = cv::imread(path);
    if (img.empty())
        return false;

    m_origin = img.clone();
    m_current = img.clone();
    m_sourcePath = filePath;
    m_savePath.Empty();

    while (!m_undoStack.empty()) m_undoStack.pop();
    while (!m_redoStack.empty()) m_redoStack.pop();

    m_loaded = true;
    return true;
}

void ImageDocument::resetToOrigin()
{
    if (!m_loaded)
        return;
    apply(m_origin);
}

void ImageDocument::clear()
{
    m_origin.release();
    m_current.release();
    m_analyze.release();
    m_reference.release();
    while (!m_undoStack.empty()) m_undoStack.pop();
    while (!m_redoStack.empty()) m_redoStack.pop();
    m_sourcePath.Empty();
    m_savePath.Empty();
    m_refPath.Empty();
    m_loaded = false;
}

void ImageDocument::apply(const cv::Mat& img)
{
    if (!m_loaded)
    {
        // 尚未载入图像时直接作为首张图（正常流程不会走到，防御性处理）
        m_origin = img.clone();
        m_current = img.clone();
        m_loaded = true;
        return;
    }

    m_undoStack.push(m_current.clone());
    while (!m_redoStack.empty()) m_redoStack.pop();
    m_current = img.clone();
}

bool ImageDocument::undo()
{
    if (!m_loaded || m_undoStack.empty())
        return false;

    m_redoStack.push(m_current.clone());
    m_current = m_undoStack.top();
    m_undoStack.pop();
    return true;
}

bool ImageDocument::redo()
{
    if (!m_loaded || m_redoStack.empty())
        return false;

    m_undoStack.push(m_current.clone());
    m_current = m_redoStack.top();
    m_redoStack.pop();
    return true;
}

size_t ImageDocument::undoDepth() const
{
    return m_undoStack.size();
}

size_t ImageDocument::redoDepth() const
{
    return m_redoStack.size();
}

void ImageDocument::setAnalyze(const cv::Mat& img)
{
    m_analyze = img.clone();
}

void ImageDocument::setReference(const cv::Mat& img)
{
    m_reference = img.clone();
}
