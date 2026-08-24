#pragma once

#include <string>
#include <vector>

namespace real_optimizer::system
{
    enum class IssueSeverity
    {
        Info,
        Warning,
        Critical
    };

    struct AnalysisIssue
    {
        std::wstring title;
        std::wstring description;

        IssueSeverity severity =
            IssueSeverity::Info;

        int scorePenalty = 0;
    };

    struct AnalysisResult
    {
        int healthScore = 100;

        std::vector<AnalysisIssue> issues;

        [[nodiscard]]
        bool HasIssues() const noexcept
        {
            return !issues.empty();
        }

        [[nodiscard]]
        std::size_t GetIssueCount() const noexcept
        {
            return issues.size();
        }
    };

    class SystemAnalyzer final
    {
    public:
        SystemAnalyzer() = delete;

        [[nodiscard]]
        static AnalysisResult Analyze();
    };
}