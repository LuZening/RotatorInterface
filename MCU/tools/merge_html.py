import re
import os
folder_data = os.path.join('..', 'data_uncomp')
folder_new_data = os.path.join('..', 'data')
re_script = re.compile(r'<script src="(.*)"')
re_css = re.compile(r'<link href="(.*?)"')
if __name__ == '__main__':
    list_html = [s for s in os.listdir(folder_data) if s.endswith('.html') or s.endswith('.htm')]
    for fname_html in list_html:
        print('processing', fname_html)
        f = open(os.path.join(folder_data, fname_html), 'r')
        s_new_file = ''
        for l in f:
            group = re_script.search(l)
            if group:
                path_js = os.path.join(folder_new_data, group[1])
                with open(path_js) as f_js:
                    print(path_js)
                    s_new_file += f'<!--{group[1]}-->\n'
                    s_new_file += '<script>\n'
                    s_new_file += f_js.read()
                    s_new_file += '</script>\n'
            elif l.find('text/css') > 0:
                group = re_css.search(l)
                if group:
                    path_css = os.path.join(folder_new_data, group[1])
                    with open(path_css) as f_css:
                        print(path_css)
                        s_new_file += f'<!--{group[1]}-->\n'
                        s_new_file += '<style>\n'
                        s_new_file += f_css.read()
                        s_new_file += '</style>\n'
            else:
                s_new_file += l
        f.close()
        s_new_file += '\n'
        with open(os.path.join(folder_new_data, fname_html), 'w') as f_new:
            f_new.write(s_new_file)
            print('done', fname_html)
