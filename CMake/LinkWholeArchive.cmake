function(LinkWholeArchive TARGET LIB)
    target_link_libraries(${TARGET}
        PRIVATE
            "$<LINK_LIBRARY:WHOLE_ARCHIVE,${LIB}>"
    )
endfunction()