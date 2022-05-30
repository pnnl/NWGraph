# SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
# SPDX-FileCopyrightText: 2022 University of Washington
#
# SPDX-License-Identifier: BSD-3-Clause

import os,sys,yaml,json,re

sources = {}

def initDict(libref):

    for (dirpath, dirnames, filenames) in os.walk(libref):

        for filename in filenames:

            if (filename.endswith('md')):
                
                local_pagename = os.path.join(dirpath, filename)

                if (filename == 'index.md'):
                    key   = dirpath # os.path.splitext(dirpath)[0]
                else:
                    key   = os.path.splitext(local_pagename)[0]


                # load just the yaml between --- and ---
                str = open(local_pagename).read()

                match = re.search(r"^-{3,}(.*?)-{3,}", str, re.M | re.DOTALL)
                str = match.group(0)

                value = list(yaml.safe_load_all(str))
                
                # print('   ====>>>  walking page: ', local_pagename)

                sources.update({key:value})


def library_library(path):
    descendents = dict(filter(lambda item:
                              item[1][0]['layout'] == 'library' and
                              item[1][0]['library-type'] == 'library',
                              path.items())) 
    
    return descendents


def library_sourcefile(path):
    descendents = dict(filter(lambda item: 
                              item[1][0]['layout'] == 'library' and
                              item[1][0]['library-type'] == 'sourcefile',
                              path.items())) 
    return descendents


def layout_library(path):
    descendents = dict(filter(lambda item: 
                              item[1][0]['layout'] == 'library',
                              path.items())) 
    return descendents

def layout_directory(path):
    descendents = dict(filter(lambda item: 
                              item[1][0]['layout'] == 'directory',
                              path.items())) 
    return descendents


def layout_enum(path):
    descendents = dict(filter(lambda item: 
                              item[1][0]['layout'] == 'enum',
                              path.items())) 
    return descendents


def layout_class(path):
    descendents = dict(filter(lambda item: 
                              item[1][0]['layout'] == 'class',
                              path.items())) 
    return descendents
    

def layout_function(path):
    descendents = dict(filter(lambda item: 
                              item[1][0]['layout'] == 'function',
                              path.items())) 
    return descendents


def layout_method(path):
    descendents = dict(filter(lambda item: 
                              item[1][0]['layout'] == 'method',
                              path.items())) 
    return descendents


def jsonize(path):
    return json.loads(path)

def basename(path):
    if path:
      return os.path.basename(path)
    else: 
      return ""

def markdownify(path):
    return path

def frob_slash(path):
    return path.replace('/', '2F')

def parent(path):
    (foo, bar) = os.path.split(os.path.dirname(path))
    return bar

def relpath(path):
    return os.path.relpath(path, 'libref')


def siblings(path):

    search_key = path

    if (os.path.basename(path) == 'index'):
        search_key = os.path.dirname(path)

    # print(':::::::))) siblings search key', search_key)

    descendents = dict(filter(lambda item: 
                              search_key == os.path.split(item[0])[0],
                              sources.items())) 

    res = descendents


    return res


def add_jinja_filters(app):
    # Make sure we're outputting HTML
    if app.builder.format != 'html':
        return    

    app.builder.templates.environment.add_extension('jinja2_highlight.HighlightExtension')

    app.builder.templates.environment.filters['jsonize']     = jsonize
    app.builder.templates.environment.filters['markdownify'] = markdownify

    app.builder.templates.environment.filters['relpath']     = relpath
    app.builder.templates.environment.filters['basename']    = basename
    app.builder.templates.environment.filters['frob_slash']  = frob_slash
    app.builder.templates.environment.filters['parent']      = parent
    app.builder.templates.environment.filters['siblings']    = siblings

    app.builder.templates.environment.filters['library_library']     = library_library
    app.builder.templates.environment.filters['library_sourcefile']   = library_sourcefile

    app.builder.templates.environment.filters['layout_directory']   = layout_directory
    app.builder.templates.environment.filters['layout_library']   = layout_library
    app.builder.templates.environment.filters['layout_enum']      = layout_enum
    app.builder.templates.environment.filters['layout_class']     = layout_class
    app.builder.templates.environment.filters['layout_function']  = layout_function
    app.builder.templates.environment.filters['layout_method']    = layout_method


def rstjinja(app, docname, source):
    """
    Render our pages as a jinja template for fancy templating goodness.
    """
    # Make sure we're outputting HTML
    if app.builder.format != 'html':
        return
    src = source[0]
    rendered = app.builder.templates.render_string(
        src, app.config.html_context
    )
    source[0] = rendered


local_html_context = {
     'sources' : sources
}


def hydeme_context():
    return local_html_context
    

def setup(app):
    '''                                                                                                                             
    Adds extra jinja filters.                                                                                                       
    '''

    print('SETTING UP HYDE')


    initDict('libref')

    app.connect("builder-inited", add_jinja_filters)
    app.connect("source-read", rstjinja)
