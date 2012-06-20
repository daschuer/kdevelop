{% load kdev_filters %}
/*

 {{ license|lines_prepend:" * " }}
 */

#include "{{ output_file_header }}"

{% include "namespace_use_cpp.txt" %}

{% for declaration in declarations %}
{% with declaration.internal_declarations as arguments %}

{% if declaration.type %}{{ declaration.type }} {% endif %}{{ name }}::{{ declaration.identifier }}({% include "arguments_types_names.txt" %})
{
    {% if declaration.type %}
    return {{ declaration.default_return_value }};
    {% endif %}
}
{% endwith %}
{% endfor %}