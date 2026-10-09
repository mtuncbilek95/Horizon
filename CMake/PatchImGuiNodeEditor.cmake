function(HorizonPatchFile path from to)
	file(READ "${path}" content)
	string(REPLACE "\r\n" "\n" content "${content}")
	string(FIND "${content}" "${to}" patched)
	if(NOT patched EQUAL -1)
		return()
	endif()
	string(FIND "${content}" "${from}" found)
	if(found EQUAL -1)
		message(FATAL_ERROR "[PATCH ERROR] -- Pattern not found in ${path}: ${from}")
	endif()
	string(REPLACE "${from}" "${to}" content "${content}")
	file(WRITE "${path}" "${content}")
endfunction()

set(OPERATOR_BODY "inline ImVec2 operator*(const float lhs, const ImVec2& rhs)\n{\n    return ImVec2(lhs * rhs.x, lhs * rhs.y);\n}\n")

HorizonPatchFile("${SOURCE_DIR}/imgui_extra_math.inl"
	"${OPERATOR_BODY}"
	"# if IMGUI_VERSION_NUM < 19268\n${OPERATOR_BODY}# endif\n"
)

HorizonPatchFile("${SOURCE_DIR}/imgui_node_editor.cpp"
	"drawList->PathStroke(color, true, strokeThickness);"
	"drawList->PathStroke(color, strokeThickness, ImDrawFlags_Closed);"
)

HorizonPatchFile("${SOURCE_DIR}/imgui_node_editor.cpp"
	"m_BorderColor, m_Rounding, m_Corners, m_BorderWidth);"
	"m_BorderColor, m_Rounding, m_BorderWidth, m_Corners);"
)

HorizonPatchFile("${SOURCE_DIR}/imgui_node_editor.cpp"
	"m_GroupBorderColor, m_GroupRounding, c_AllRoundCornersFlags, m_GroupBorderWidth);"
	"m_GroupBorderColor, m_GroupRounding, m_GroupBorderWidth, c_AllRoundCornersFlags);"
)

HorizonPatchFile("${SOURCE_DIR}/imgui_node_editor.cpp"
	"color, ImMax(0.0f, m_Rounding + offset), c_AllRoundCornersFlags, thickness);"
	"color, ImMax(0.0f, m_Rounding + offset), thickness, c_AllRoundCornersFlags);"
)