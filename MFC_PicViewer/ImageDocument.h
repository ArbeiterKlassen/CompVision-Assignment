#pragma once

#include "pch.h"
#include "framework.h"
#include <opencv2/opencv.hpp>
#include <stack>

// ImageDocument
//
// 集中管理程序中的图像状态，取代原先散落在 MFC_PicViewerDlg.cpp 顶部的
// 全局变量（origin_img / current_img / Analyze_img / referrence_img /
// img_stack / file_path 等）。
//
// 撤销/重做语义：
//   - m_undoStack 保存历史快照（不含当前图），m_redoStack 保存撤销回收的快照
//   - open()   : 载入新图，清空两个栈，current = origin
//   - apply()  : 当前图入撤销栈，清空重做栈，current = 新图
//   - undo()   : 当前图入重做栈，current = 撤销栈顶
//   - redo()   : 当前图入撤销栈，current = 重做栈顶

class ImageDocument
{
public:
    ImageDocument();

    // ---- 状态查询 ----
    bool hasImage() const;                 // 是否已成功载入图像

    const cv::Mat& origin() const;         // 原始图（载入时快照）
    const cv::Mat& current() const;        // 当前工作图
    const cv::Mat& analyze() const;        // 分析结果图（直方图等）
    const cv::Mat& reference() const;      // 参考图（直方图规定化用）

    // ---- 文件路径 ----
    const CString& sourcePath() const;     // 当前图像源文件
    void setSourcePath(const CString& path);

    const CString& savePath() const;       // 最近一次保存目录推导的路径
    void setSavePath(const CString& path);

    const CString& referencePath() const;  // 参考图路径
    void setReferencePath(const CString& path);

    // ---- 载入 / 重置 / 清空 ----
    bool open(const CString& filePath);    // imread + 重置全部状态
    void resetToOrigin();                  // 恢复原始图（可撤销）
    void clear();                          // 清空全部图像状态

    // ---- 编辑 ----
    void apply(const cv::Mat& img);        // 提交新图像（可撤销），并清空重做栈

    // ---- 撤销 / 重做 ----
    bool undo();                           // 成功返回 true，无可撤销时返回 false
    bool redo();                           // 成功返回 true，无可重做时返回 false
    size_t undoDepth() const;
    size_t redoDepth() const;

    // ---- 分析结果 / 参考图 ----
    void setAnalyze(const cv::Mat& img);
    void setReference(const cv::Mat& img);

private:
    cv::Mat m_origin;
    cv::Mat m_current;
    cv::Mat m_analyze;
    cv::Mat m_reference;

    std::stack<cv::Mat> m_undoStack;
    std::stack<cv::Mat> m_redoStack;

    CString m_sourcePath;
    CString m_savePath;
    CString m_refPath;

    bool m_loaded = false;
};
